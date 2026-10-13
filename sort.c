/*
 * sort.c - Comparison functions and sorting of entry lists.
 *
 * Phase 3: lexicographic order by name, reversible with -r.
 * (Sorting by time and size is added in phase 4.)
 */
#include <stdlib.h>
#include <string.h>

#include "sort.h"

/*
 * qsort(3) gives the comparison function no extra argument, so the options
 * in effect are kept in a file-scope pointer set by sort_entries().
 */
static const struct options *sort_opt;

/* Compare two entries by name in lexicographical (byte) order. */
static int
compare_names(const void *a, const void *b)
{
	const struct entry *x = a;
	const struct entry *y = b;
	int result = strcmp(x->name, y->name);

	return sort_opt->reverse ? -result : result;
}

void
sort_entries(struct entry_list *list, const struct options *opt)
{
	if (opt->no_sort || list->count < 2)
		return;

	sort_opt = opt;
	qsort(list->items, list->count, sizeof(struct entry), compare_names);
}
