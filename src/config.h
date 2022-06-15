#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "args.h"

struct config {

	// Name by which the binary was called.
	const char *name;

	// Path of the dictionary file to use.
	const char *dictfile;

	// Words given on the command line.
	struct args words;

	// All words in the anagram must have at least this length.
	uint8_t minlength;

	// The anagram must contain at least one word of this length.
	uint8_t haslength;

	// Whether the user requested the help message.
	bool print_help;
};

// Default program config.
extern const struct config config_default;
