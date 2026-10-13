/*
 * list.h - High level listing logic: handling of the operands and of the
 *          directories they name.
 */
#ifndef LIST_H
#define LIST_H

#include "options.h"

/*
 * List the given operands (argv[0..argc-1]).  With no operands the current
 * directory is listed.
 *
 * Returns 0 if everything was listed, 1 if at least one error occurred.
 */
int list_operands(int argc, char **argv, const struct options *opt);

#endif /* LIST_H */
