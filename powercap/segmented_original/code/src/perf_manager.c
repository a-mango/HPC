#include "perf_manager.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *enable_cmd  = "enable";
static const char *disable_cmd = "disable";
static const char *ack_cmd     = "ack\n";

// Send commands to perf via FIFOs
static void send_command(PerfManager *pm, const char *command) {
    if (pm->enable) {
        write(pm->ctl_fd, command, strlen(command));
        char ack[5];
        read(pm->ack_fd, ack, 5);
        assert(strcmp(ack, ack_cmd) == 0);
    }
}

// Initialize PerfManager with FIFO file descriptors
void PerfManager_init(PerfManager *pm) {
    char *ctl_fd_env = getenv("PERF_CTL_FD");
    char *ack_fd_env = getenv("PERF_ACK_FD");
    pm->enable       = (ctl_fd_env && ack_fd_env);
    if (pm->enable) {
        pm->ctl_fd = atoi(ctl_fd_env);
        pm->ack_fd = atoi(ack_fd_env);
    } else {
        pm->ctl_fd = -1;
        pm->ack_fd = -1;
    }
}

// Pause/resume energy measurement
void PerfManager_pause(PerfManager *pm) {
    send_command(pm, disable_cmd);
}
void PerfManager_resume(PerfManager *pm) {
    send_command(pm, enable_cmd);
}
