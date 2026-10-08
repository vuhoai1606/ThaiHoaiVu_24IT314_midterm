#include <stdio.h>
#include <stdlib.h>
#include "options.h"
#include "traverse.h"

int main(int argc, char *argv[]) {
    Options opts;
    init_options(&opts);

    int opt_index = parse_options(argc, argv, &opts);
    if (opt_index < 0) {
        return 1;
    }

    char **operands = &argv[opt_index];
    int operand_count = argc - opt_index;

    int status = process_operands(operands, operand_count, &opts);

    return (status == 0) ? EXIT_SUCCESS : 1;
}
