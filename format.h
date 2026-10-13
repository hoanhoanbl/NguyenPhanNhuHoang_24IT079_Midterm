/*
 * format.h - Conversion of file attributes to the text shown by ls:
 *            mode string, date, sizes, -F indicator.
 */
#ifndef FORMAT_H
#define FORMAT_H

#include <stddef.h>
#include <sys/stat.h>
#include <time.h>

#include "options.h"

/* Size of the buffer needed by format_mode(): 10 characters + '\0'. */
#define MODE_LEN 11

/*
 * Write the 10 character mode string of -l (e.g. "drwxr-xr-x") for the
 * given status.  buf must hold at least MODE_LEN bytes.
 */
void format_mode(const struct stat *st, char *buf);

/*
 * Write the date shown by -l: "Mon DD HH:MM" for files modified within
 * about six months of now, "Mon DD  YYYY" otherwise.
 */
void format_time(time_t t, char *buf, size_t size);

/*
 * Write a count of 512-byte disk blocks as shown by -s and by the "total"
 * line.  The unit is 1024 with -k, otherwise the BLOCKSIZE environment
 * variable (512 by default); partial units are rounded up.  (With -h the
 * caller uses format_bytes() instead, as -h reports sizes in bytes.)
 */
void format_blocks(long long blocks512, const struct options *opt,
    char *buf, size_t size);

/* Write a size in bytes: plain number, or human readable with -h. */
void format_bytes(long long bytes, const struct options *opt,
    char *buf, size_t size);

/*
 * Return the character -F appends to a name ('/', '*', '@', '%', '=' or
 * '|'), or '\0' when the file needs none.
 */
char classify_char(const struct stat *st);

#endif /* FORMAT_H */
