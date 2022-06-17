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

#include "config.h"
#include "histogram.h"
#include "input.h"

struct word {
	char *str;
	size_t len;
	struct histogram *hist;
	struct word *next;
};

struct prev_word {
	char *str;
	size_t len;
	struct prev_word *prev;
};

static struct word *word_head = NULL;
static struct word *word_tail = NULL;

static void
usage (const struct config *config)
{
	char *usage[] = {
		"  -h|--help                  Show this help text",
		"  -f|--dictfile <dictfile>   Use this dictionary file (one word per line)",
		"  -m|--minlength <length>    All anagram words must be at least this long",
		"  -l|--haslength <length>    One anagram word must be at least this long\n"
	};
	unsigned int i;

	fprintf(stderr, "\nFind anagrams of the input phrases (as argument, else standard input)\n");
	fprintf(stderr, "Usage: %s [-h] [-f dictfile] [-m minlength] [-l haslength] words...\n\n", config->name);

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
words_find (const struct config *config, struct histogram *h, struct prev_word *prev, int len_satisfied)
{
	struct word *w;
	struct histogram *copy;
	struct prev_word pw;
	struct prev_word *p;

	/* If the anagram must contain a word of a minimum length, which has
	 * not occurred so far, and there are not enough letters left in the
	 * histogram to create words of that length, abort this branch: */
	if (!len_satisfied && h->ntotal < config->haslength) {
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
			if (len_satisfied || w->len >= config->haslength) {
				fwrite(w->str, w->len, 1, stdout);
				/* Backtrack over all previous words in this chain,
				 * print them all in reverse order: */
				for (p = prev; p; p = p->prev) {
					fputc(' ', stdout);
					fwrite(p->str, p->len, 1, stdout);
				}
				fputc('\n', stdout);
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
		pw.len = w->len;
		pw.prev = prev;
		words_find(config, copy, &pw, (len_satisfied || w->len >= config->haslength));
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

static int
char_compare (const void *const p1, const void *const p2)
{
	if (*(char *const)p1 == *(char *const)p2) {
		return 0;
	}
	return (*(char *const)p1 < *(char *const)p2) ? -1 : 1;
}

static int
parse_dictfile (const struct config *config, const struct histogram *const inhist, size_t *max_found_len, size_t *nwords)
{
	FILE *fp = NULL;
	char buf[10000];	/* Window chunk size, not max filesize */
	char *c;
	char *anchor;
	size_t nbytes;
	size_t len = 0;
	int skip_word = 0;

	if ((fp = fopen(config->dictfile, "r")) == NULL) {
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
			if (skip_word == 0 && len >= config->minlength) {
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
	struct config config = config_default;
	struct input  input;
	struct histogram *inhist;
	size_t max_found_len = 0;
	size_t nwords = 0;

	// Parse the command line options.
	if (!args_parse(&config, &(struct args) { .ac = argc, .av = argv })) {
		usage(&config);
		return 1;
	}

	// Check if the user just wants the help message.
	if (config.print_help) {
		usage(&config);
		return 0;
	}

	// Get the input string from the command line arguments or stdin.
	if (!input_get(&config, &input)) {
		return 1;
	}

	/* Create histogram of input string: */
	if ((inhist = histogram_create(input.str, input.len)) == NULL) {
		fprintf(stderr, "Could not create histogram of input\n");
		free(input.str);
		return 1;
	}
	/* Parse the dictionary file: */
	if (parse_dictfile(&config, inhist, &max_found_len, &nwords) == 0) {
		fprintf(stderr, "Could not parse file\n");
		histogram_destroy(&inhist);
		free(input.str);
		return 1;
	}
	/* Check that we have words, and at least one has a length of at least
	 * 'anagram_contains_len': */
	if (max_found_len >= config.haslength && nwords > 0) {
		words_find(&config, inhist, NULL, 0);
	}
	words_destroy();
	histogram_destroy(&inhist);
	free(input.str);
	return 0;
}
