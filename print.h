/*
 * print.h - Output of lists of entries on standard output.
 */
#ifndef PRINT_H
#define PRINT_H

#include "entry.h"
#include "options.h"

/*
 * Print every entry of the list, one per line, in the format selected by
 * the options (-i, -s, -l, -n, -F, -h, -k, -q, -w).  The columns are
 * aligned using the widest value of each column.
 *
 * is_directory is non-zero when the list is the content of a directory:
 * the "total" line of -l (and of -s on a terminal) is then printed first.
 */
void print_entries(const struct entry_list *list, const struct options *opt,
    int is_directory);

#endif /* PRINT_H */
