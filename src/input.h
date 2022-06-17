#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "config.h"

struct input {

	// Pointer to a heap-allocated copy of the input string, to be free'd
	// by the owner of this structure.
	char *str;

	// Length of the input string in bytes.
	size_t len;
};

// Get the input string from the command line arguments, or from standard input
// if no arguments were given.
extern bool input_get (const struct config *config, struct input *input);
