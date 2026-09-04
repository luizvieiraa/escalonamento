#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scheduler.h"

static int parse_algorithm(const char *argument, SchedulerAlgorithm *algorithm)
{
    if (strcmp(argument, "rate") == 0) {
        *algorithm = SCHEDULER_RATE;
        return 1;
    }

    if (strcmp(argument, "edf") == 0) {
        *algorithm = SCHEDULER_EDF;
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    SchedulerAlgorithm algorithm;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <rate|edf> <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (!parse_algorithm(argv[1], &algorithm)) {
        fprintf(stderr,
                "Error: invalid algorithm '%s'; use 'rate' or 'edf'.\n",
                argv[1]);
        return EXIT_FAILURE;
    }

    return scheduler_run(algorithm, argv[2]);
}
