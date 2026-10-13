/*
 * print.c - Printing of entries: short format, -i, -s, -F and the long
 *           format of -l / -n.
 */
#include <ctype.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "format.h"
#include "print.h"
#include "util.h"

/* Text of every column of one output line (filled before printing). */
struct row {
	char inode[24];
	char blocks[32];
	char mode[MODE_LEN];
	char nlink[24];
	char owner[64];
	char group[64];
	char size[40];
	char major[24];		/* device files: major number */
	char minor[24];		/* device files: minor number */
	char date[40];
	char *target;		/* symbolic link: pointed-to path, or NULL */
	int is_device;		/* size column holds "major, minor" */
};

/* Width (number of characters) of the widest value of each column. */
struct widths {
	int inode, blocks, nlink, owner, group, size, major, minor;
};

/*
 * Print a file name.  With -q (or by default on a terminal) every
 * non-printable character is replaced by '?'; with -w it is written as is.
 */
static void
print_name(const char *name, const struct options *opt)
{
	const unsigned char *p;

	for (p = (const unsigned char *)name; *p != '\0'; p++) {
		if (opt->nonprint == NONPRINT_QUESTION && !isprint(*p))
			putchar('?');
		else
			putchar(*p);
	}
}

/* Read the target of a symbolic link into allocated memory (NULL on error). */
static char *
read_link(const char *path)
{
	char buf[PATH_MAX + 1];
	ssize_t len;

	len = readlink(path, buf, sizeof(buf) - 1);
	if (len < 0) {
		report_error(path);
		return NULL;
	}
	buf[len] = '\0';
	return xstrdup(buf);
}

/* Owner column: user name, or the number if unknown or with -n. */
static void
fill_owner(char *buf, size_t size, uid_t uid, const struct options *opt)
{
	struct passwd *pw = opt->numeric_ids ? NULL : getpwuid(uid);

	if (pw != NULL)
		snprintf(buf, size, "%s", pw->pw_name);
	else
		snprintf(buf, size, "%lu", (unsigned long)uid);
}

/* Group column: group name, or the number if unknown or with -n. */
static void
fill_group(char *buf, size_t size, gid_t gid, const struct options *opt)
{
	struct group *gr = opt->numeric_ids ? NULL : getgrgid(gid);

	if (gr != NULL)
		snprintf(buf, size, "%s", gr->gr_name);
	else
		snprintf(buf, size, "%lu", (unsigned long)gid);
}

/* Compute the text of every column needed by the options for one entry. */
static void
fill_row(const struct entry *e, const struct options *opt, struct row *r)
{
	const struct stat *st = &e->st;

	memset(r, 0, sizeof(*r));

	if (opt->inode)
		snprintf(r->inode, sizeof(r->inode), "%llu",
		    (unsigned long long)st->st_ino);
	if (opt->blocks) {
		/*
		 * With -h the -s column is human readable: in the short
		 * format it shows the size in bytes, in the long format the
		 * space used on disk (as the system ls does).
		 */
		if (opt->size_kind == SIZE_HUMAN && !opt->long_format)
			format_bytes((long long)st->st_size, opt, r->blocks,
			    sizeof(r->blocks));
		else if (opt->size_kind == SIZE_HUMAN)
			format_bytes((long long)st->st_blocks * 512, opt,
			    r->blocks, sizeof(r->blocks));
		else
			format_blocks((long long)st->st_blocks, opt,
			    r->blocks, sizeof(r->blocks));
	}
	if (!opt->long_format)
		return;

	format_mode(st, r->mode);
	snprintf(r->nlink, sizeof(r->nlink), "%lu",
	    (unsigned long)st->st_nlink);
	fill_owner(r->owner, sizeof(r->owner), st->st_uid, opt);
	fill_group(r->group, sizeof(r->group), st->st_gid, opt);

	/* Device files show "major, minor" instead of a size. */
	if (S_ISCHR(st->st_mode) || S_ISBLK(st->st_mode)) {
		r->is_device = 1;
		snprintf(r->major, sizeof(r->major), "%lu",
		    (unsigned long)major(st->st_rdev));
		snprintf(r->minor, sizeof(r->minor), "%lu",
		    (unsigned long)minor(st->st_rdev));
	} else
		format_bytes((long long)st->st_size, opt, r->size,
		    sizeof(r->size));

	format_time(entry_time(e, opt->time_kind).tv_sec, r->date,
	    sizeof(r->date));

	if (S_ISLNK(st->st_mode))
		r->target = read_link(e->path);
}

/* Raise *width to the length of s if s is longer. */
static void
widen(int *width, const char *s)
{
	int len = (int)strlen(s);

	if (len > *width)
		*width = len;
}

/* Find the width of each column over all rows. */
static void
compute_widths(const struct row *rows, size_t count, struct widths *w)
{
	size_t i;

	memset(w, 0, sizeof(*w));
	for (i = 0; i < count; i++) {
		widen(&w->inode, rows[i].inode);
		widen(&w->blocks, rows[i].blocks);
		widen(&w->nlink, rows[i].nlink);
		widen(&w->owner, rows[i].owner);
		widen(&w->group, rows[i].group);
		if (rows[i].is_device) {
			widen(&w->major, rows[i].major);
			widen(&w->minor, rows[i].minor);
		} else
			widen(&w->size, rows[i].size);
	}

	/* The "major, minor" pair shares the size column. */
	if (w->major > 0 && w->major + 2 + w->minor > w->size)
		w->size = w->major + 2 + w->minor;
}

/* Print one output line. */
static void
print_row(const struct entry *e, const struct row *r,
    const struct widths *w, const struct options *opt)
{
	char indicator;

	if (opt->inode)
		printf("%*s ", w->inode, r->inode);
	if (opt->blocks)
		printf("%*s ", w->blocks, r->blocks);

	if (opt->long_format) {
		printf("%s  %*s %-*s  %-*s  ", r->mode, w->nlink, r->nlink,
		    w->owner, r->owner, w->group, r->group);
		if (r->is_device)
			printf("%*s, %*s ", w->size - 2 - w->minor, r->major,
			    w->minor, r->minor);
		else
			printf("%*s ", w->size, r->size);
		printf("%s ", r->date);
	}

	print_name(e->name, opt);
	if (opt->classify && (indicator = classify_char(&e->st)) != '\0')
		putchar(indicator);

	if (r->target != NULL) {
		fputs(" -> ", stdout);
		print_name(r->target, opt);
	}
	putchar('\n');
}

/*
 * Print the "total N" line: blocks used by all the listed files, or with -h
 * the sum of their sizes in bytes (consistent with the -s column).
 */
static void
print_total(const struct entry_list *list, const struct options *opt)
{
	long long blocks = 0, bytes = 0;
	char text[40];
	size_t i;

	for (i = 0; i < list->count; i++) {
		blocks += (long long)list->items[i].st.st_blocks;
		bytes += (long long)list->items[i].st.st_size;
	}
	if (opt->size_kind == SIZE_HUMAN)
		format_bytes(bytes, opt, text, sizeof(text));
	else
		format_blocks(blocks, opt, text, sizeof(text));
	printf("total %s\n", text);
}

void
print_entries(const struct entry_list *list, const struct options *opt,
    int is_directory)
{
	struct widths w;
	struct row *rows;
	size_t i;

	/* -l always shows the total; -s only when writing to a terminal. */
	if (is_directory &&
	    (opt->long_format || (opt->blocks && isatty(STDOUT_FILENO))))
		print_total(list, opt);

	if (list->count == 0)
		return;

	rows = xmalloc(list->count * sizeof(struct row));
	for (i = 0; i < list->count; i++)
		fill_row(&list->items[i], opt, &rows[i]);
	compute_widths(rows, list->count, &w);

	for (i = 0; i < list->count; i++)
		print_row(&list->items[i], &rows[i], &w, opt);

	for (i = 0; i < list->count; i++)
		free(rows[i].target);
	free(rows);
}
