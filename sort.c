/*
 * sort.c - Comparison functions and sorting of entry lists.
 *
 * Sort keys, in order of priority:
 *   -S  size, largest first
 *   -t  time (modification, -c change or -u access), newest first
 *   default: name, lexicographical order
 * Entries with an equal key are ordered by name.  -r reverses the whole
 * order, and -f disables sorting altogether.
 */
#include <stdlib.h>
#include <string.h>

#include "sort.h"

/*
 * qsort(3) gives the comparison function no extra argument, so the options
 * in effect are kept in a file-scope pointer set by sort_entries().
 */
static const struct options *sort_opt;

/* Compare by size: negative when x is larger (largest first). */
static int
compare_sizes(const struct entry *x, const struct entry *y)
{
	if (x->st.st_size > y->st.st_size)
		return -1;
	if (x->st.st_size < y->st.st_size)
		return 1;
	return 0;
}

/* Compare by selected time: negative when x is newer (newest first). */
static int
compare_times(const struct entry *x, const struct entry *y)
{
	struct timespec tx = entry_time(x, sort_opt->time_kind);
	struct timespec ty = entry_time(y, sort_opt->time_kind);

	if (tx.tv_sec != ty.tv_sec)
		return tx.tv_sec > ty.tv_sec ? -1 : 1;
	if (tx.tv_nsec != ty.tv_nsec)
		return tx.tv_nsec > ty.tv_nsec ? -1 : 1;
	return 0;
}

/* Full comparison used by qsort(): chosen key, then name, then -r. */
static int
compare_entries(const void *a, const void *b)
{
	const struct entry *x = a;
	const struct entry *y = b;
	int result = 0;

	if (sort_opt->sort_size)
		result = compare_sizes(x, y);
	else if (sort_opt->sort_time)
		result = compare_times(x, y);

	/* Equal keys (or plain name sort): lexicographical order. */
	if (result == 0)
		result = strcmp(x->name, y->name);

	return sort_opt->reverse ? -result : result;
}

void
sort_entries(struct entry_list *list, const struct options *opt)
{
	if (opt->no_sort || list->count < 2)
		return;

	sort_opt = opt;
	qsort(list->items, list->count, sizeof(struct entry),
	    compare_entries);
}
