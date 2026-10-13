/*
 * print.c - Printing of entries.
 *
 * Phase 3: only the name is printed, one per line.  The extra fields
 * (-i, -s, -l, -F, ...) are added in phase 5.
 */
#include <ctype.h>
#include <stdio.h>

#include "print.h"

/*
 * Print a file name.  With -q (or by default on a terminal) every
 * non-printable character is replaced by '?'; with -w it is written as is.
 */
static void
print_name(const char *name, const struct options *opt)
{
	const unsigned char *p;

	for (p = (const unsigned char *)name; *p != '\0'; p++) {
		if (opt->nonprint == NONPRINT_QUESTION && !isprint(*p))
			putchar('?');
		else
			putchar(*p);
	}
}

void
print_entry(const struct entry *e, const struct options *opt)
{
	print_name(e->name, opt);
	putchar('\n');
}
