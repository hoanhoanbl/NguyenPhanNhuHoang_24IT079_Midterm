/*
 * options.h - Command line options of ls (subset given in the manual):
 *             -AacdFfhiklnqRrSstuw
 */
#ifndef OPTIONS_H
#define OPTIONS_H

/* Which timestamp is used for sorting (-t) and printing (-l). */
enum time_kind {
	TIME_MTIME,	/* last modification (default) */
	TIME_CTIME,	/* last status change (-c)     */
	TIME_ATIME	/* last access (-u)            */
};

/* How sizes are shown with -s and -l. */
enum size_kind {
	SIZE_DEFAULT,	/* blocks / bytes                     */
	SIZE_KILO,	/* kilobytes (-k)                     */
	SIZE_HUMAN	/* human readable, e.g. 1.5K (-h)     */
};

/* How non-printable characters in file names are shown. */
enum nonprint_kind {
	NONPRINT_DEFAULT,	/* '?' on a terminal, raw otherwise */
	NONPRINT_QUESTION,	/* always '?' (-q)                  */
	NONPRINT_RAW		/* always raw (-w)                  */
};

struct options {
	int all;		/* -a: include names starting with '.'   */
	int almost_all;		/* -A: all except "." and ".."           */
	int plain_dirs;		/* -d: list directories as plain files   */
	int classify;		/* -F: append type indicator to names    */
	int no_sort;		/* -f: do not sort                       */
	int inode;		/* -i: print inode number                */
	int long_format;	/* -l or -n: long listing                */
	int numeric_ids;	/* -n: show uid/gid as numbers           */
	int recursive;		/* -R: list subdirectories recursively   */
	int reverse;		/* -r: reverse the sort order            */
	int sort_size;		/* -S: sort by size, largest first       */
	int blocks;		/* -s: print allocated blocks            */
	int sort_time;		/* -t: sort by time, newest first        */
	enum time_kind time_kind;
	enum size_kind size_kind;
	enum nonprint_kind nonprint;
};

/*
 * Parse argv with getopt(3) and fill *opt.  Options that override each
 * other (-l/-n, -c/-u, -q/-w, -S/-t, -k/-h) are resolved so that the last
 * one given wins.  Two behaviours follow the system ls rather than the
 * manual text: -f implies -a, and -d beats -R in any order.
 *
 * Returns the index in argv of the first operand (file name), or -1 if an
 * unknown option was given.
 */
int parse_options(int argc, char **argv, struct options *opt);

/* Print the usage message to stderr. */
void print_usage(void);

#endif /* OPTIONS_H */
