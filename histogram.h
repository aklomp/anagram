#ifndef HISTOGRAM_H
#define HISTOGRAM_H

struct histogram {
	char *bins;	/* Pointer to array holding characters */
	int *freq;	/* Pointer to array holding frequency counts */
	size_t len;	/* Number of bins (unique characters) */
	int maxfreq;	/* Maximum frequency found in this histogram */
	size_t ntotal;	/* Total characters represented (length of source string) */
};

struct histogram *histogram_create (const char *const str, const size_t len);
struct histogram *histogram_copy (const struct histogram *orig);
void histogram_destroy (struct histogram **h);
int histogram_fits (const struct histogram *const test, const struct histogram *const base);
int histogram_subtract (struct histogram *target, struct histogram *from);

#endif /* HISTOGRAM_H */
