#include <stdio.h>
#include "../histogram.h"

#define ASSERT(x) if (!(x)) printf("FAILED: line %d\n", __LINE__)

int
main ()
{
	struct histogram *ha, *hb, *hc, *hd, *he, *hf;

	ha = histogram_create("abc", 3);
	hb = histogram_create("aaa", 3);
	hc = histogram_create("a", 1);
	hd = histogram_create("", 0);
	he = histogram_create("abcabc", 6);
	hf = histogram_create("abcabcabc", 9);

	ASSERT(ha->len == 3);
	ASSERT(ha->maxfreq == 1);
	ASSERT(ha->ntotal == 3);

	ASSERT(hb->len == 1);
	ASSERT(hb->maxfreq == 3);
	ASSERT(hb->ntotal == 3);

	ASSERT(hc->len == 1);
	ASSERT(hc->maxfreq == 1);
	ASSERT(hc->ntotal == 1);

	ASSERT(hd->len == 0);
	ASSERT(hd->maxfreq == 0);
	ASSERT(hd->ntotal == 0);

	ASSERT(he->len == 3);
	ASSERT(he->maxfreq == 2);
	ASSERT(he->ntotal == 6);

	ASSERT(hf->len == 3);
	ASSERT(hf->maxfreq == 3);
	ASSERT(hf->ntotal == 9);

	/* Histograms should fit themselves: */
	ASSERT(histogram_fits(ha, ha) == 1);
	ASSERT(histogram_fits(hb, hb) == 1);
	ASSERT(histogram_fits(hc, hc) == 1);
	ASSERT(histogram_fits(hd, hd) == 1);
	ASSERT(histogram_fits(he, he) == 1);
	ASSERT(histogram_fits(hf, hf) == 1);

	/* No three a's in 'abc': */
	ASSERT(histogram_fits(ha, hb) == 0);

	/* Three a's in 'abcabcabc': */
	ASSERT(histogram_fits(ha, hf) == 1);

	/* Single letter a fits many: */
	ASSERT(histogram_fits(hc, ha) == 1);
	ASSERT(histogram_fits(hc, hb) == 1);
	ASSERT(histogram_fits(hc, he) == 1);
	ASSERT(histogram_fits(hc, hf) == 1);

	/* Empty histogram fits all: */
	ASSERT(histogram_fits(hd, ha) == 1);
	ASSERT(histogram_fits(hd, hb) == 1);
	ASSERT(histogram_fits(hd, hc) == 1);
	ASSERT(histogram_fits(hd, hd) == 1);
	ASSERT(histogram_fits(hd, he) == 1);
	ASSERT(histogram_fits(hd, hf) == 1);

	/* 'abc' fits 'abcabc', but 'abcabc' does not fit 'abc': */
	ASSERT(histogram_fits(ha, he) == 1);
	ASSERT(histogram_fits(he, ha) == 0);

	/* 'abcabc' fits 'abcabcabc': */
	ASSERT(histogram_fits(he, hf) == 1);
	ASSERT(histogram_fits(hf, he) == 0);

	/* Subtracting 'abc' from 'abcabcabc': */
	ASSERT(histogram_subtract(hf, ha) == 1);
	ASSERT(hf->len == 3);
	ASSERT(hf->maxfreq == 2);
	ASSERT(hf->ntotal == 6);

	/* Subtracting 'abcabc' from 'abcabc': */
	ASSERT(histogram_subtract(hf, he) == 1);
	/* 3 fields left of frequency 0 each: */
	ASSERT(hf->len == 3);
	ASSERT(hf->freq[0] == 0);
	ASSERT(hf->freq[1] == 0);
	ASSERT(hf->freq[2] == 0);
	ASSERT(hf->ntotal == 0);

	return 0;
}
