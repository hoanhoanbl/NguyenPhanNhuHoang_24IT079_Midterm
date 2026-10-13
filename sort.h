/*
 * sort.h - Ordering of the entries before they are printed.
 */
#ifndef SORT_H
#define SORT_H

#include "entry.h"
#include "options.h"

/*
 * Sort the list in place according to the options.
 * Does nothing when -f (no sorting) is in effect.
 */
void sort_entries(struct entry_list *list, const struct options *opt);

#endif /* SORT_H */
