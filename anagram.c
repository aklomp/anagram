/* anagram: find anagrams of the input text
 * Copyright (C) 2013  Alfred Klomp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <unistd.h>	/* read() */
#include <stdio.h>
#include <stdlib.h>	/* malloc() */
#include <string.h>	/* memmove() */
#include <errno.h>
#include <getopt.h>
#include <ctype.h>	/* isspace() */
#include "histogram.h"

#define DEFAULT_DICTFILE	"/usr/share/dict/words"

struct word {
	char *str;
	size_t len;
	struct histogram *hist;
	struct word *next;
};

struct prev_word {
	char *str;
	struct prev_word *prev;
};

static struct word *word_head = NULL;
static struct word *word_tail = NULL;

static void
usage (const char *prgnam)
{
	char *usage[] = {
		"  -h|--help                  Show this help text",
		"  -f|--wordfile <dictfile>   Use this dictionary file (one word per line)",
		"  -m|--minlength <length>    All anagram words must be at least this long",
		"  -l|--haslength <length>    One anagram word must be at least this long\n"
	};
	unsigned int i;

	fprintf(stderr, "\nFind anagrams of the input phrases (as argument, else standard input)\n");
	fprintf(stderr, "Usage: %s [-h] [-f dictfile] [-m minlength] [-l haslength] words...\n\n", prgnam);

	for (i = 0; i < sizeof(usage) / sizeof(usage[0]); i++) {
		fprintf(stderr, "%s\n", usage[i]);
	}
}

static int
word_add (const char *const word, const size_t len, const struct histogram *const inhist)
{
	struct word *w;
	struct histogram *h;

	if (len == 0) {
		goto err_0;
	}
	/* If the word is longer than the input string, the word is out: */
	if (len > inhist->ntotal) {
		goto err_0;
	}
	if ((h = histogram_create(word, len)) == NULL) {
		goto err_0;
	}
	/* If the word's histogram has more max occurrences than the input
	 * histogram, the word cannot be part of the anagram (cheap check): */
	if (h->maxfreq > inhist->maxfreq) {
		goto err_1;
	}
	/* Further compare histograms; if the word has a higher occurrence
	 * count for any given character than the input, then the word is out: */
	if (!histogram_fits(h, inhist)) {
		goto err_1;
	}
	if ((w = malloc(sizeof(*w))) == NULL) {
		goto err_1;
	}
	if ((w->str = malloc(len + 1)) == NULL) {
		goto err_2;
	}
	w->len = len;
	w->next = NULL;
	memcpy(w->str, word, len);
	w->str[len] = '\0';
	w->hist = h;

	/* Add word to linked list: */
	if (word_head == NULL) {
		word_head = word_tail = w;
	}
	else {
		word_tail->next = w;
		word_tail = w;
	}
	return 1;

err_2:	free(w);
err_1:	histogram_destroy(&h);
err_0:	return 0;
}

static void
words_find (struct histogram *h, struct prev_word *prev, size_t anagram_contains_len, int len_satisfied)
{
	struct word *w;
	struct histogram *copy;
	struct prev_word pw;
	struct prev_word *p;

	/* If the anagram must contain a word of a minimum length, which has
	 * not occurred so far, and there are not enough letters left in the
	 * histogram to create words of that length, abort this branch: */
	if (!len_satisfied && h->ntotal < anagram_contains_len) {
		return;
	}
	/* Loop over all words; anagram may contain the same word more than once: */
	for (w = word_head; w; w = w->next)
	{
		/* Skip word if longer than there are characters in the histogram: */
		if (w->len > h->ntotal) {
			continue;
		}
		/* Skip word if its histogram does not fit into input histogram: */
		if (histogram_fits(w->hist, h) == 0) {
			continue;
		}
		if ((copy = histogram_copy(h)) == NULL) {
			return;
		}
		if (histogram_subtract(copy, w->hist) == 0) {
			histogram_destroy(&copy);
			return;
		}
		/* Empty histogram? Done! */
		if (copy->ntotal == 0)
		{
			/* Ensure before printing that at least one of the words in
			 * the anagram is at least 'anagram_contains_len' in length: */
			if (len_satisfied || w->len >= anagram_contains_len) {
				printf("%s", w->str);
				/* Backtrack over all previous words in this chain,
				 * print them all in reverse order: */
				for (p = prev; p; p = p->prev) {
					printf(" %s", p->str);
				}
				printf("\n");
			}
			histogram_destroy(&copy);
			return;
		}
		/* Else recurse: */

		/* Do something cute here: fill a prev_word structure on the
		 * stack with the current word and current value of prev_word.
		 * When a branch terminates, it backtracks over this list to
		 * print out all the words in the chain leading up to it. We
		 * essentially create a list on the stack of breadcrumbs of
		 * where we came from: */
		pw.str = w->str;
		pw.prev = prev;
		words_find(copy, &pw, anagram_contains_len, (len_satisfied || w->len >= anagram_contains_len));
		histogram_destroy(&copy);
	}
}

static void
words_destroy (void)
{
	struct word *w;
	struct word *t;

	for (w = word_head; w; w = t) {
		t = w->next;
		histogram_destroy(&w->hist);
		free(w->str);
		free(w);
	}
}

static char *
get_instr_from_args (int argc, char *argv[], size_t *inlen)
{
	size_t len = 0;
	int i;
	char *c;
	char *p;
	char *instr;

	/* No options left on command line? */
	if (optind >= argc) {
		return NULL;
	}
	/* Find combined length of input strings without spaces: */
	for (i = optind; i < argc; i++) {
		for (c = argv[i]; *c; c++) {
			if (!isspace(*c)) {
				len++;
			}
		}
	}
	/* Allocate a string of this size: */
	if (len == 0 || (p = instr = malloc(*inlen = len)) == NULL) {
		return NULL;
	}
	/* Now copy the input strings, minus whitespace,
	 * to this string: */
	for (i = optind; i < argc; i++) {
		for (c = argv[i]; *c; c++) {
			if (!isspace(*c)) {
				*p++ = *c;
			}
		}
	}
	return instr;
}

static char *
get_instr_from_stdin (size_t *inlen)
{
	char *instr = NULL;
	ssize_t n, len = 0;
	char *t, *h;

	/* Copy characters char by char from stdin to buffer.
	 * We allocate a max buffer size of 100 characters, which is extremely
	 * generous, since the amount of anagrams explodes with input size: */
	if ((instr = malloc(100)) == NULL) {
		return NULL;
	}
	do {
		/* Keep retrying the read in case of EINTR: */
		while ((n = read(0, instr, 100 - len)) < 0 && errno == EINTR) {
			;
		}
		len += n;
	}
	while (n > 0 && len < 100);

	if (len == 0) {
		free(instr);
		return NULL;
	}
	/* Now remove all spaces: */
	for (t = h = instr; *h && h - instr < len; h++) {
		if (!isspace(*h)) {
			*t++ = *h;
		}
	}
	*t = '\0';
	*inlen = t - instr;
	return instr;
}

static int
char_compare (const void *const p1, const void *const p2)
{
	if (*(char *const)p1 == *(char *const)p2) {
		return 0;
	}
	return (*(char *const)p1 < *(char *const)p2) ? -1 : 1;
}

static int
parse_dictfile (const char *const filename, const struct histogram *const inhist, size_t minlen, size_t *max_found_len, size_t *nwords)
{
	FILE *fp = NULL;
	char buf[10000];	/* Window chunk size, not max filesize */
	char *c;
	char *anchor;
	size_t nbytes;
	size_t len = 0;
	int skip_word = 0;

	if ((fp = fopen(filename, "r")) == NULL) {
		return 0;
	}
	while ((nbytes = len + fread(buf + len, 1, sizeof(buf) - len, fp)) > 0)
	{
		/* Split on newlines: */
		for (anchor = c = buf; c < buf + nbytes; c++) {
			if (*c != '\n')
			{
				/* If already skipping this word, don't inspect the char: */
				if (skip_word) {
					continue;
				}
				/* Check if this char is in the list of chars in the
				 * input string; if not, this word can never be part
				 * of the anagram: */
				if (bsearch(c, inhist->bins, inhist->len, 1, char_compare)) {
					len++;
				}
				else {
					skip_word = 1;
				}
				continue;
			}
			if (skip_word == 0 && len >= minlen) {
				if (word_add(anchor, len, inhist)) {
					if (len > *max_found_len) {
						*max_found_len = len;
					}
					(*nwords)++;
				}
			}
			anchor = c + 1;
			len = 0;
			skip_word = 0;
		}
		/* Move remainder to front of buffer: */
		if (len > 0) {
			memmove(buf, anchor, len);
		}
	}
	fclose(fp);
	return 1;
}

int
main (int argc, char *argv[])
{
	struct histogram *inhist;
	size_t word_min_len = 1;
	size_t anagram_contains_len = 1;
	char *filename = DEFAULT_DICTFILE;
	char *instr;
	size_t inlen;
	size_t max_found_len = 0;
	size_t nwords = 0;

	/* Parse options: */
	for (;;)
	{
		int c;
		int opt_index = 0;
		static struct option opt_long[] = {
			{ "help", 0, 0, 'h' },
			{ "wordfile", 1, 0, 'f' },
			{ "minlength", 1, 0, 'm' },
			{ "haslength", 1, 0, 'l' },
			{ 0, 0, 0, 0 }
		};
		if ((c = getopt_long(argc, argv, "hf:m:l:", opt_long, &opt_index)) == -1) {
			break;
		}
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return 0;

			case 'f':
				/* Dictionary file to use (file must contain one word per line): */
				filename = optarg;
				break;

			case 'm':
				/* All words in the anagram must have at least this length: */
				word_min_len = atoi(optarg);
				break;

			case 'l':
				/* The anagram must contain a word of at least this length: */
				anagram_contains_len = atoi(optarg);
				break;
		}
	}
	/* Interpret the other elements as input strings to anagram;
	 * if no further arguments, read from stdin: */
	if ((instr = get_instr_from_args(argc, argv, &inlen)) == NULL
	 && (instr = get_instr_from_stdin(&inlen)) == NULL) {
		/* The anagram of the empty string is the empty string. Success? */
		return 0;
	}
	/* Create histogram of input string: */
	if ((inhist = histogram_create(instr, inlen)) == NULL) {
		fprintf(stderr, "Could not create histogram of input\n");
		return 1;
	}
	/* Parse the dictionary file: */
	if (parse_dictfile(filename, inhist, word_min_len, &max_found_len, &nwords) == 0) {
		fprintf(stderr, "Could not parse file\n");
		histogram_destroy(&inhist);
		return 1;
	}
	/* Check that we have words, and at least one has a length of at least
	 * 'anagram_contains_len': */
	if (max_found_len >= anagram_contains_len && nwords > 0) {
		words_find(inhist, NULL, anagram_contains_len, 0);
	}
	words_destroy();
	histogram_destroy(&inhist);
	free(instr);
	return 0;
}
