#pragma once

#include <stdbool.h>
#include <stddef.h>

struct histogram {

	// Pointer to the array holding the characters.
	char *bins;

	// Pointer to the array holding the frequency counts.
	int *freq;

	// Number of bins (unique characters).
	size_t len;

	// Maximum character frequency found in this histogram.
	int maxfreq;

	// Total characters represented (length of source string).
	size_t ntotal;
};

extern struct histogram *histogram_create (const char *str, const size_t len);
extern struct histogram *histogram_copy (const struct histogram *orig);
extern void histogram_destroy (struct histogram **h);
extern bool histogram_fits (const struct histogram *test, const struct histogram *base);
extern bool histogram_subtract (struct histogram *target, struct histogram *from);
