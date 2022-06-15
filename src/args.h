#pragma once

#include <stdbool.h>

// Forward declaration to avoid an include loop.
struct config;

struct args {

	// Pointer to a list of argument strings.
	char **av;

	// Number of arguments in #av.
	int ac;
};

// Parse the command line arguments, populate the config structure.
extern bool args_parse (struct config *config, const struct args *args);
