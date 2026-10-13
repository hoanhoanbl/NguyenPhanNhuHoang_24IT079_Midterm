/*
 * options.c - Command line parsing for ls.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "options.h"
#include "util.h"

/* Option letters accepted by this implementation. */
#define OPTSTRING "AacdFfhiklnqRrSstuw"

int
parse_options(int argc, char **argv, struct options *opt)
{
	int ch;

	/* Start from defaults: everything off, mtime, sizes in blocks. */
	memset(opt, 0, sizeof(*opt));
	opt->time_kind = TIME_MTIME;
	opt->size_kind = SIZE_DEFAULT;
	opt->nonprint = NONPRINT_DEFAULT;

	while ((ch = getopt(argc, argv, OPTSTRING)) != -1) {
		switch (ch) {
		case 'A': opt->almost_all = 1; break;
		case 'a': opt->all = 1; break;
		case 'F': opt->classify = 1; break;
		case 'f': opt->no_sort = 1; break;
		case 'i': opt->inode = 1; break;
		case 'r': opt->reverse = 1; break;
		case 'S': opt->sort_size = 1; break;
		case 's': opt->blocks = 1; break;
		case 't': opt->sort_time = 1; break;

		/* -c and -u override each other: last one wins. */
		case 'c': opt->time_kind = TIME_CTIME; break;
		case 'u': opt->time_kind = TIME_ATIME; break;

		/* -l and -n override each other: last one wins. */
		case 'l':
			opt->long_format = 1;
			opt->numeric_ids = 0;
			break;
		case 'n':
			opt->long_format = 1;
			opt->numeric_ids = 1;
			break;

		/* -q and -w override each other: last one wins. */
		case 'q': opt->nonprint = NONPRINT_QUESTION; break;
		case 'w': opt->nonprint = NONPRINT_RAW; break;

		/* -R and -d override each other: last one wins. */
		case 'R':
			opt->recursive = 1;
			opt->plain_dirs = 0;
			break;
		case 'd':
			opt->plain_dirs = 1;
			opt->recursive = 0;
			break;

		/* -k and -h override each other: rightmost flag wins. */
		case 'k': opt->size_kind = SIZE_KILO; break;
		case 'h': opt->size_kind = SIZE_HUMAN; break;

		default:	/* getopt already printed the error */
			return -1;
		}
	}

	/* -A is always set for the super-user. */
	if (geteuid() == 0)
		opt->almost_all = 1;

	/* Resolve the default for non-printable characters. */
	if (opt->nonprint == NONPRINT_DEFAULT)
		opt->nonprint = isatty(STDOUT_FILENO) ?
		    NONPRINT_QUESTION : NONPRINT_RAW;

	return optind;
}

void
print_usage(void)
{
	fprintf(stderr, "usage: %s [-%s] [file ...]\n", progname, OPTSTRING);
}
