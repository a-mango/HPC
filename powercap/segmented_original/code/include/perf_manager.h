#ifndef PERF_MANAGER_H
#define PERF_MANAGER_H

#include <stdbool.h>

typedef struct {
    int  ctl_fd;
    int  ack_fd;
    bool enable;
} PerfManager;

void PerfManager_init(PerfManager *pm);
void PerfManager_pause(PerfManager *pm);
void PerfManager_resume(PerfManager *pm);

#endif
