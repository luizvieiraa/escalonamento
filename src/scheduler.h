#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef enum {
    SCHEDULER_RATE,
    SCHEDULER_EDF
} SchedulerAlgorithm;

int scheduler_run(SchedulerAlgorithm algorithm, const char *input_path);

#endif
