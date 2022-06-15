#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "args.h"
#include "config.h"

static bool
get_uint8 (uint8_t *dst)
{
	char *eptr;
	const long l = strtol(optarg, &eptr, 10);

	if (l <= 0 || l > 255 || *eptr != '\0') {
		return false;
	}

	*dst = (uint8_t) l;
	return true;
}

bool
args_parse (struct config *config, const struct args *args)
{
	int c;
	static const struct option opts[] = {
		{ "help",      no_argument,       NULL, 'h' },
		{ "dictfile",  required_argument, NULL, 'f' },
		{ "minlength", required_argument, NULL, 'm' },
		{ "haslength", required_argument, NULL, 'l' },
		{ NULL }
	};

	// Sanity checks.
	if (config == NULL || args == NULL) {
		return false;
	}

	// Save the name by which the program was called.
	config->name = args->av[0];

	// Parse the command line options.
	while ((c = getopt_long(args->ac, args->av, ":hf:m:l:", opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			config->print_help = true;
			return true;

		case 'f':
			config->dictfile = optarg;
			break;

		case 'm':
			if (!get_uint8(&config->minlength)) {
				fprintf(stderr, "%s: '%s': invalid value.\n",
				        config->name, optarg);
				return false;
			}
			break;

		case 'l':
			if (!get_uint8(&config->haslength)) {
				fprintf(stderr, "%s: '%s': invalid value.\n",
				        config->name, optarg);
				return false;
			}
			break;

		default:
			if (optopt != 0) {
				fprintf(stderr, "%s: '%c': unknown option.\n",
				        config->name, optopt);
			} else {
				fprintf(stderr, "%s: %s: unknown option.\n",
				        config->name, args->av[optind - 1]);
			}
			return false;
		}
	}

	// The positional arguments are the words to anagram.
	config->words.ac = args->ac - optind;
	config->words.av = args->av + optind;

	return true;
}
