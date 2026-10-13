/*
 * util.h - Small helper functions shared by all modules:
 *          checked memory allocation and error reporting.
 */
#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

/* Program name used as the prefix of every error message. */
extern const char *progname;

/* malloc() that terminates the program if memory is exhausted. */
void *xmalloc(size_t size);

/* realloc() that terminates the program if memory is exhausted. */
void *xrealloc(void *ptr, size_t size);

/* strdup() that terminates the program if memory is exhausted. */
char *xstrdup(const char *s);

/*
 * Print "progname: path: <strerror(errno)>" to stderr.
 * Call it right after a failed system call so errno is still valid.
 */
void report_error(const char *path);

#endif /* UTIL_H */
