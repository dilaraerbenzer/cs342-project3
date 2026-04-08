#ifndef RSM_H
#define RSM_H

#include <pthread.h>

#define MAX_RT 100 // max num of resource types supported
#define MAX_PR 100 // max num of processes supported

int rsm_init(int p_count, int r_count,
            int exist[], int avoid);
int rsm_destroy();
int rsm_process_started(int apid);
int rsm_process_ended();
int rsm_claim (int claim[]); // only for avoidance
int rsm_request (int request[]);
int rsm_release (int release[]);
int rsm_detection();
void rsm_print_state (char headermsg[]);

#endif /* RSM_H */
