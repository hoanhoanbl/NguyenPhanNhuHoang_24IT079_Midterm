/*
 * entry.h - A directory entry (or command line operand) together with its
 *           file status, and a growable list of such entries.
 */
#ifndef ENTRY_H
#define ENTRY_H

#include <stddef.h>
#include <sys/stat.h>

struct entry {
	char *name;		/* name shown to the user              */
	char *path;		/* path used to access the file        */
	struct stat st;		/* status of the file                  */
};

struct entry_list {
	struct entry *items;	/* dynamically allocated array         */
	size_t count;		/* number of entries in use            */
	size_t capacity;	/* number of entries allocated         */
};

/* Make *list an empty list. */
void entry_list_init(struct entry_list *list);

/*
 * Append a new entry to the list.  name, path and st are copied, so the
 * caller keeps ownership of its arguments.
 */
void entry_list_add(struct entry_list *list, const char *name,
    const char *path, const struct stat *st);

/* Release all memory held by the list and leave it empty. */
void entry_list_free(struct entry_list *list);

#endif /* ENTRY_H */
