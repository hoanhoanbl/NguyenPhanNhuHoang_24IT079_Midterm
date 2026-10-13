/*
 * format.c - Conversion of file attributes to text.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "format.h"

/* About six months in seconds: the limit between "recent" and "old" files. */
#define SIX_MONTHS ((time_t)182 * 24 * 60 * 60)

/* BLOCKSIZE limits and default, in bytes. */
#define MIN_BLOCK_SIZE 512ULL
#define MAX_BLOCK_SIZE (1024ULL * 1024 * 1024)

/* ------------------------------------------------------------------ */
/* Mode string                                                         */
/* ------------------------------------------------------------------ */

/* Return the character describing the type of the entry. */
static char
type_char(mode_t m)
{
	/* NetBSD "archive state" bits come first (not a real file type). */
#ifdef S_ARCH2
	if (m & S_ARCH2)
		return 'A';
#endif
#ifdef S_ARCH1
	if (m & S_ARCH1)
		return 'a';
#endif
	if (S_ISDIR(m))
		return 'd';
	if (S_ISLNK(m))
		return 'l';
	if (S_ISBLK(m))
		return 'b';
	if (S_ISCHR(m))
		return 'c';
	if (S_ISSOCK(m))
		return 's';
	if (S_ISFIFO(m))
		return 'p';
#ifdef S_ISWHT
	if (S_ISWHT(m))
		return 'w';
#endif
	return '-';
}

/*
 * Third character of a permission triplet.  "special" is the set-id (or
 * sticky) bit: it turns 'x' into with_x and '-' into without_x.
 */
static char
exec_char(int executable, int special, char with_x, char without_x)
{
	if (special)
		return executable ? with_x : without_x;
	return executable ? 'x' : '-';
}

void
format_mode(const struct stat *st, char *buf)
{
	mode_t m = st->st_mode;

	buf[0] = type_char(m);
	buf[1] = (m & S_IRUSR) ? 'r' : '-';
	buf[2] = (m & S_IWUSR) ? 'w' : '-';
	buf[3] = exec_char(m & S_IXUSR, m & S_ISUID, 's', 'S');
	buf[4] = (m & S_IRGRP) ? 'r' : '-';
	buf[5] = (m & S_IWGRP) ? 'w' : '-';
	buf[6] = exec_char(m & S_IXGRP, m & S_ISGID, 's', 'S');
	buf[7] = (m & S_IROTH) ? 'r' : '-';
	buf[8] = (m & S_IWOTH) ? 'w' : '-';
	buf[9] = exec_char(m & S_IXOTH, m & S_ISVTX, 't', 'T');
	buf[10] = '\0';
}

/* ------------------------------------------------------------------ */
/* Date                                                                */
/* ------------------------------------------------------------------ */

void
format_time(time_t t, char *buf, size_t size)
{
	struct tm tm;
	time_t now = time(NULL);
	const char *fmt;

	if (localtime_r(&t, &tm) == NULL) {
		snprintf(buf, size, "?");
		return;
	}

	/* Recent (past or near future): show the time; otherwise the year. */
	if (t > now - SIX_MONTHS && t < now + SIX_MONTHS)
		fmt = "%b %e %H:%M";
	else
		fmt = "%b %e  %Y";

	if (strftime(buf, size, fmt, &tm) == 0)
		buf[0] = '\0';
}

/* ------------------------------------------------------------------ */
/* Sizes                                                               */
/* ------------------------------------------------------------------ */

/* Write bytes as a short human readable string: 512B, 1.5K, 23M, ... */
static void
human_size(unsigned long long bytes, char *buf, size_t size)
{
	static const char units[] = "BKMGTPE";
	unsigned long long whole = bytes, rem = 0, tenths = 0;
	int unit = 0;
	int decimal;

	/* Divide by 1024 until the number is small, remembering the rest. */
	while (whole >= 1024 && unit < 6) {
		rem = whole % 1024;
		whole /= 1024;
		unit++;
	}

	if (unit == 0) {
		snprintf(buf, size, "%lluB", whole);
		return;
	}

	/* One decimal for values below 10, otherwise round to an integer. */
	if (whole >= 10) {
		if (rem >= 512)
			whole++;
		decimal = 0;
	} else {
		tenths = (rem * 10 + 512) / 1024;
		if (tenths >= 10) {
			whole++;
			tenths = 0;
		}
		decimal = 1;
	}

	/* Rounding may have produced 1024 of the unit: move to the next. */
	if (whole >= 1024 && unit < 6) {
		whole = 1;
		tenths = 0;
		decimal = 1;
		unit++;
	}

	if (decimal)
		snprintf(buf, size, "%llu.%llu%c", whole, tenths, units[unit]);
	else
		snprintf(buf, size, "%llu%c", whole, units[unit]);
}

/*
 * Read the BLOCKSIZE environment variable: a number, optionally followed
 * by k, m or g.  Invalid or out of range values fall back to 512 / the
 * limits, as getbsize(3) does.
 */
static unsigned long long
env_block_size(void)
{
	const char *env = getenv("BLOCKSIZE");
	char *end;
	unsigned long long value;

	if (env == NULL || *env == '\0')
		return MIN_BLOCK_SIZE;

	value = strtoull(env, &end, 10);
	if (end == env)
		return MIN_BLOCK_SIZE;
	if (value > MAX_BLOCK_SIZE)
		value = MAX_BLOCK_SIZE;

	switch (*end) {
	case 'k': case 'K': value *= 1024; end++; break;
	case 'm': case 'M': value *= 1024 * 1024; end++; break;
	case 'g': case 'G': value *= 1024ULL * 1024 * 1024; end++; break;
	default: break;
	}
	if (*end != '\0')
		return MIN_BLOCK_SIZE;

	if (value < MIN_BLOCK_SIZE)
		value = MIN_BLOCK_SIZE;
	if (value > MAX_BLOCK_SIZE)
		value = MAX_BLOCK_SIZE;
	return value;
}

void
format_blocks(long long blocks512, const struct options *opt,
    char *buf, size_t size)
{
	unsigned long long bytes, unit;

	if (blocks512 < 0)
		blocks512 = 0;
	bytes = (unsigned long long)blocks512 * 512;

	unit = (opt->size_kind == SIZE_KILO) ? 1024ULL : env_block_size();

	/* Round partial units up. */
	snprintf(buf, size, "%llu", (bytes + unit - 1) / unit);
}

void
format_bytes(long long bytes, const struct options *opt, char *buf,
    size_t size)
{
	if (bytes < 0)
		bytes = 0;
	if (opt->size_kind == SIZE_HUMAN)
		human_size((unsigned long long)bytes, buf, size);
	else
		snprintf(buf, size, "%llu", (unsigned long long)bytes);
}

/* ------------------------------------------------------------------ */
/* -F indicator                                                        */
/* ------------------------------------------------------------------ */

char
classify_char(const struct stat *st)
{
	mode_t m = st->st_mode;

	if (S_ISDIR(m))
		return '/';
	if (S_ISLNK(m))
		return '@';
#ifdef S_ISWHT
	if (S_ISWHT(m))
		return '%';
#endif
	if (S_ISSOCK(m))
		return '=';
	if (S_ISFIFO(m))
		return '|';
	if (S_ISREG(m) && (m & (S_IXUSR | S_IXGRP | S_IXOTH)))
		return '*';
	return '\0';
}
