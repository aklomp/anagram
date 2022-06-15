#include "config.h"

// Default program config.
const struct config config_default = {
	.name       = "anagram",
	.dictfile   = "/usr/share/dict/words",
	.minlength  = 1,
	.haslength  = 1,
	.print_help = false,
};
