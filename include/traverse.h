#ifndef TRAVERSE_H
#define TRAVERSE_H

#include "options.h"

/*
 * Process all command-line operands (or "." if none provided).
 * Returns exit status: 0 on total success, >0 if any error occurred.
 */
int process_operands(char *operands[], int operand_count, const Options *opts);

#endif /* TRAVERSE_H */
