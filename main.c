/*
 * main.c - Entry point of the ls clone.
 *
 * TEMPORARY (phase 2): only parses the options and prints them so the
 * parser can be tested.  Replaced in phase 3 by the real listing code.
 */
#include <stdio.h>
#include <stdlib.h>

#include "options.h"
#include "util.h"

int
main(int argc, char **argv)
{
	struct options opt;
	int first, i;

	if (argc > 0 && argv[0] != NULL)
		progname = argv[0];

	first = parse_options(argc, argv, &opt);
	if (first < 0) {
		print_usage();
		return 1;
	}

	printf("a=%d A=%d d=%d F=%d f=%d i=%d l=%d n=%d R=%d r=%d S=%d s=%d t=%d\n",
	    opt.all, opt.almost_all, opt.plain_dirs, opt.classify, opt.no_sort,
	    opt.inode, opt.long_format, opt.numeric_ids, opt.recursive,
	    opt.reverse, opt.sort_size, opt.blocks, opt.sort_time);
	printf("time=%d size=%d nonprint=%d\n",
	    opt.time_kind, opt.size_kind, opt.nonprint);
	for (i = first; i < argc; i++)
		printf("operand: %s\n", argv[i]);
	return 0;
}
