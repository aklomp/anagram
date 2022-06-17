#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "input.h"

// Max number of characters to copy from standard input. This value is very
// generous because the amount of anagrams explodes with the input size.
#define STDIN_SIZE	100

static bool
input_from_stdin (struct input *input)
{
	ssize_t nread;
	char *t, *p;

	// Reset the result structure.
	input->len = 0;

	// Copy at most STDIN_SIZE characters from standard input.
	if ((p = input->str = malloc(STDIN_SIZE)) == NULL) {
		return false;
	}

	while (input->len < STDIN_SIZE) {

		// Read input from stdin, handle errors.
		if ((nread = read(0, p, STDIN_SIZE - input->len)) < 0) {
			if (errno == EINTR) {
				continue;
			}
			break;
		}

		// Handle EOF.
		if (nread == 0) {
			break;
		}

		// A positive amount of bytes was read.
		input->len += (size_t) nread;
		p          += (size_t) nread;
	}

	// Remove all whitespace from the input.
	for (char *h = t = input->str; h < input->str + input->len; h++) {
		if (!isspace(*h)) {
			*t++ = *h;
		}
	}

	// Return false if nothing was read.
	if (input->len == 0 || t == input->str) {
		free(input->str);
		input->str = NULL;
		return false;
	}

	*t = '\0';
	input->len = t - input->str;
	return true;
}

static bool
input_from_args (const struct config *config, struct input *input)
{
	char *p;

	// Reset the result structure.
	input->str = NULL;
	input->len = 0;

	// Return if the user did not specify any words on the command line.
	if (config->words.ac == 0) {
		return false;
	}

	// Find the combined length of all words without spaces.
	for (int i = 0; i < config->words.ac; i++) {
		for (char *c = config->words.av[i]; *c; c++) {
			if (!isspace(*c)) {
				input->len++;
			}
		}
	}

	// Return if the input is empty.
	if (input->len == 0) {
		return false;
	}

	// Allocate a string of this size.
	if ((p = input->str = malloc(input->len)) == NULL) {
		return false;
	}

	// Copy the input strings, minus whitespace.
	for (int i = 0; i < config->words.ac; i++) {
		for (char *c = config->words.av[i]; *c; c++) {
			if (!isspace(*c)) {
				*p++ = *c;
			}
		}
	}

	return true;
}

bool
input_get (const struct config *config, struct input *input)
{
	// Try to get the input string from the command line arguments.
	if (input_from_args(config, input)) {
		return true;
	}

	// Try to get the input string from standard input.
	if (input_from_stdin(input)) {
		return true;
	}

	return false;
}
