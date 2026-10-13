/*
 * main.c - Entry point of the ls clone.
 */
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "options.h"
#include "util.h"

int
main(int argc, char **argv)
{
	struct options opt;
	const char *slash;
	int first, status;

	/* Use only the last component of argv[0] in messages. */
	if (argc > 0 && argv[0] != NULL) {
		slash = strrchr(argv[0], '/');
		progname = slash != NULL ? slash + 1 : argv[0];
	}

	first = parse_options(argc, argv, &opt);
	if (first < 0) {
		print_usage();
		return 1;
	}

	status = list_operands(argc - first, argv + first, &opt);

	/* A failed write (e.g. closed pipe, full disk) is an error too. */
	if (fflush(stdout) != 0 || ferror(stdout)) {
		report_error("stdout");
		status = 1;
	}
	return status;
}
