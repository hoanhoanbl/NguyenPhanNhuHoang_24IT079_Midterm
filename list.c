/*
 * list.c - Listing of operands and directory contents.
 */
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "entry.h"
#include "list.h"
#include "print.h"
#include "sort.h"
#include "util.h"

/* Non-zero once something has been written (decides blank-line separators). */
static int printed_output;

/* Build "dir/name" in newly allocated memory (no doubled slash). */
static char *
join_path(const char *dir, const char *name)
{
	size_t dirlen = strlen(dir);
	size_t namelen = strlen(name);
	int need_slash = (dirlen > 0 && dir[dirlen - 1] != '/');
	char *path = xmalloc(dirlen + need_slash + namelen + 1);

	memcpy(path, dir, dirlen);
	if (need_slash)
		path[dirlen] = '/';
	memcpy(path + dirlen + need_slash, name, namelen + 1);
	return path;
}

/*
 * Decide whether a name found in a directory is listed:
 *   - normal names are always listed;
 *   - names starting with '.' need -a, or -A (which excludes "." and "..").
 */
static int
is_listed(const char *name, const struct options *opt)
{
	if (name[0] != '.')
		return 1;
	if (opt->all)
		return 1;
	if (opt->almost_all)
		return strcmp(name, ".") != 0 && strcmp(name, "..") != 0;
	return 0;
}

/* Sort a list and print it (is_directory: it is the content of a directory). */
static void
print_list(struct entry_list *list, const struct options *opt,
    int is_directory)
{
	sort_entries(list, opt);
	print_entries(list, opt, is_directory);
}

/*
 * List the contents of one directory.  When show_header is set, the
 * directory name followed by ':' is printed first.
 * Returns 0 on success, 1 if the directory could not be read.
 */
static int
list_directory(const char *dirpath, const struct options *opt, int show_header)
{
	struct entry_list list;
	struct dirent *de;
	DIR *dp;
	int status = 0;

	/* The header comes first, even if the directory cannot be opened. */
	if (show_header) {
		if (printed_output)
			putchar('\n');
		printf("%s:\n", dirpath);
	}
	printed_output = 1;

	dp = opendir(dirpath);
	if (dp == NULL) {
		report_error(dirpath);
		return 1;
	}

	entry_list_init(&list);
	for (;;) {
		struct stat st;
		char *path;

		/* readdir() returns NULL both at the end and on error. */
		errno = 0;
		de = readdir(dp);
		if (de == NULL)
			break;
		if (!is_listed(de->d_name, opt))
			continue;

		path = join_path(dirpath, de->d_name);
		if (lstat(path, &st) != 0) {
			report_error(path);
			status = 1;
		} else
			entry_list_add(&list, de->d_name, path, &st);
		free(path);
	}
	if (errno != 0) {
		report_error(dirpath);
		status = 1;
	}
	closedir(dp);

	print_list(&list, opt, 1);

	/*
	 * -R: list the subdirectories after this directory.  Entries come
	 * from lstat(), so symbolic links to directories are not followed
	 * (no infinite loops); "." and ".." are never entered.
	 */
	if (opt->recursive) {
		size_t i;

		for (i = 0; i < list.count; i++) {
			const struct entry *e = &list.items[i];

			if (!S_ISDIR(e->st.st_mode) ||
			    strcmp(e->name, ".") == 0 ||
			    strcmp(e->name, "..") == 0)
				continue;
			if (list_directory(e->path, opt, 1) != 0)
				status = 1;
		}
	}

	entry_list_free(&list);
	return status;
}

int
list_operands(int argc, char **argv, const struct options *opt)
{
	static char *default_operand[] = { ".", NULL };
	struct entry_list files, dirs;
	int status = 0;
	int show_header;
	int i;

	/* No operand: list the current directory. */
	if (argc == 0) {
		argc = 1;
		argv = default_operand;
	}

	entry_list_init(&files);
	entry_list_init(&dirs);

	/* Split the operands into non-directories and directories. */
	for (i = 0; i < argc; i++) {
		struct stat st;
		int rc;

		/*
		 * With -d, -l (or -n) and -F a symbolic link operand is not
		 * followed: the link itself is listed.  Otherwise it is
		 * followed, falling back to the link if its target is missing.
		 */
		if (opt->plain_dirs || opt->long_format || opt->classify)
			rc = lstat(argv[i], &st);
		else {
			rc = stat(argv[i], &st);
			if (rc != 0)
				rc = lstat(argv[i], &st);
		}
		if (rc != 0) {
			report_error(argv[i]);
			status = 1;
			continue;
		}

		if (S_ISDIR(st.st_mode) && !opt->plain_dirs)
			entry_list_add(&dirs, argv[i], argv[i], &st);
		else
			entry_list_add(&files, argv[i], argv[i], &st);
	}

	/* Non-directory operands come first. */
	if (files.count > 0) {
		print_list(&files, opt, 0);
		printed_output = 1;
	}

	/*
	 * Directory names are shown as headers with several operands.
	 * With -R the subdirectories always get one (see list_directory).
	 */
	show_header = (argc > 1);
	sort_entries(&dirs, opt);
	for (i = 0; i < (int)dirs.count; i++)
		if (list_directory(dirs.items[i].path, opt, show_header) != 0)
			status = 1;

	entry_list_free(&files);
	entry_list_free(&dirs);
	return status;
}
