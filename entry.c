/*
 * entry.c - Growable list of directory entries.
 */
#include <stdlib.h>

#include "entry.h"
#include "util.h"

/* Number of slots allocated the first time an entry is added. */
#define INITIAL_CAPACITY 16

void
entry_list_init(struct entry_list *list)
{
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
}

void
entry_list_add(struct entry_list *list, const char *name, const char *path,
    const struct stat *st)
{
	struct entry *e;

	/* Double the array when it is full (amortised O(1) append). */
	if (list->count == list->capacity) {
		size_t newcap = list->capacity ?
		    list->capacity * 2 : INITIAL_CAPACITY;

		list->items = xrealloc(list->items,
		    newcap * sizeof(struct entry));
		list->capacity = newcap;
	}

	e = &list->items[list->count++];
	e->name = xstrdup(name);
	e->path = xstrdup(path);
	e->st = *st;
}

void
entry_list_free(struct entry_list *list)
{
	size_t i;

	for (i = 0; i < list->count; i++) {
		free(list->items[i].name);
		free(list->items[i].path);
	}
	free(list->items);
	entry_list_init(list);
}
