/*
 * util.c - Implementation of the helper functions declared in util.h.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

const char *progname = "ls";

/* Print a fatal "out of memory" message and exit. */
static void
die_no_memory(void)
{
	fprintf(stderr, "%s: out of memory\n", progname);
	exit(2);
}

void *
xmalloc(size_t size)
{
	void *p;

	/* malloc(0) may legally return NULL, so always ask for >= 1 byte. */
	p = malloc(size ? size : 1);
	if (p == NULL)
		die_no_memory();
	return p;
}

void *
xrealloc(void *ptr, size_t size)
{
	void *p;

	p = realloc(ptr, size ? size : 1);
	if (p == NULL)
		die_no_memory();
	return p;
}

char *
xstrdup(const char *s)
{
	size_t len = strlen(s) + 1;
	char *copy = xmalloc(len);

	memcpy(copy, s, len);
	return copy;
}

void
report_error(const char *path)
{
	/* Save errno: fprintf could overwrite it before strerror runs. */
	int saved = errno;

	fprintf(stderr, "%s: %s: %s\n", progname, path, strerror(saved));
}
