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

#include <stdlib.h>	/* malloc() */
#include <string.h>	/* memmove() */
#include "histogram.h"

static int
char_compare (const void *const p1, const void *const p2)
{
	return (*(char *const)p1 < *(char *const)p2) ? -1 : 1;
}

struct histogram *
histogram_create (const char *str, const size_t len)
{
	char *c;
	char *p;
	char *end;
	void *tmp;
	struct histogram *h;

	if ((h = malloc(sizeof(*h))) == NULL) {
		goto err_0;
	}
	/* Worst-case length of h->bins is same as input: */
	if ((h->bins = malloc(len)) == NULL) {
		goto err_1;
	}
	if ((h->freq = malloc(len * sizeof(*h->freq))) == NULL) {
		goto err_2;
	}
	h->len = 0;
	h->maxfreq = 0;

	/* Copy input string to histogram bins area: */
	memcpy(h->bins, str, len);

	/* Alphabetically sort chars in string: */
	qsort(h->bins, len, 1, char_compare);

	/* Total number of characters tallied in histogram must be equal
	 * to length of input string: */
	h->ntotal = len;

	p = c = h->bins;
	end = h->bins + len;

	while (c < end) {
		char q = *c;
		int freq = 0;
		while (c < end && *c == q) {
			freq++;
			c++;
		}
		*p++ = q;
		h->freq[h->len++] = freq;
		if (freq > h->maxfreq) {
			h->maxfreq = freq;
		}
	}
	/* Trim memory blocks to fit: */
	if (h->len < len) {
		if ((tmp = realloc(h->bins, h->len)) != NULL) {
			h->bins = tmp;
		}
		if ((tmp = realloc(h->freq, h->len * sizeof(*h->freq))) != NULL) {
			h->freq = tmp;
		}
	}
	return h;

err_2:	free(h->bins);
err_1:	free(h);
err_0:	return NULL;
}

struct histogram *
histogram_copy (const struct histogram *orig)
{
	struct histogram *copy;

	if ((copy = malloc(sizeof(*copy))) == NULL) {
		goto err_0;
	}
	memcpy(copy, orig, sizeof(*orig));
	if ((copy->bins = malloc(orig->len)) == NULL) {
		goto err_1;
	}
	memcpy(copy->bins, orig->bins, orig->len);
	if ((copy->freq = malloc(orig->len * sizeof(*copy->freq))) == NULL) {
		goto err_2;
	}
	memcpy(copy->freq, orig->freq, orig->len * sizeof(*orig->freq));
	return copy;

err_2:	free(copy->bins);
err_1:	free(copy);
err_0:	return NULL;
}

void
histogram_destroy (struct histogram **h)
{
	if (h == NULL || *h == NULL) {
		return;
	}
	free((*h)->bins);
	free((*h)->freq);
	free(*h);
	*h = NULL;
}

static inline const char *
find_character (const struct histogram *h, const char c)
{
	for (char *b = h->bins; b < h->bins + h->len; b++) {
		if (*b > c) {
			return NULL;
		}
		if (*b == c) {
			return b;
		}
	}

	return NULL;
}

bool
histogram_fits (const struct histogram *h, const struct histogram *base)
{
	// The histogram cannot fit in the base histogram if it contains
	// characters that the base does not.
	if (h->len > base->len) {
		return false;
	}

	// The histogram cannot fit in the base histogram if it has a larger
	// maximum character frequency.
	if (h->maxfreq > base->maxfreq) {
		return false;
	}

	// If any one of the characters occurs more frequently in the histogram
	// than in the base, the histogram cannot fit.
	for (const char *b, *t = h->bins; t < h->bins + h->len; t++) {

		// Find the character in the base; quit if not found.
		if ((b = find_character(base, *t)) == NULL) {
			return false;
		}

		// The histogram cannot fit if the character has a higher
		// frequency than it has in the base.
		if (h->freq[t - h->bins] > base->freq[b - base->bins]) {
			return false;
		}
	}

	return true;
}

bool
histogram_subtract (struct histogram *target, struct histogram *from)
{
	char *t;
	char *f;
	int *freq_t;
	int *freq_f;

	/* Subtract the second histogram from the first. Modifies the first
	 * histogram. Returns 1 on success, 0 on "underflow". */

	target->maxfreq = 0;

	for (f = from->bins, t = target->bins; f < from->bins + from->len; f++) {
		freq_t = target->freq + (t - target->bins);

		/* Locate the corresponding char in target: */
		while (*t < *f) {
			if (*freq_t > target->maxfreq) {
				target->maxfreq = *freq_t;
			}
			if (++t == target->bins + target->len) {
				return false;
			}
			freq_t = target->freq + (t - target->bins);
		}
		/* Corresponding char not found? Quit: */
		if (*t != *f) {
			return false;
		}
		/* Helper variable, for readability: */
		freq_f = from->freq + (f - from->bins);

		/* Underflow? */
		if (*freq_f > *freq_t) {
			return false;
		}
		*freq_t -= *freq_f;
		target->ntotal -= *freq_f;
		if (*freq_t > target->maxfreq) {
			target->maxfreq = *freq_t;
		}
	}
	return true;
}
