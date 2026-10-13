/*
 * print.h - Output of one entry on standard output.
 */
#ifndef PRINT_H
#define PRINT_H

#include "entry.h"
#include "options.h"

/* Print one entry followed by a newline. */
void print_entry(const struct entry *e, const struct options *opt);

#endif /* PRINT_H */
