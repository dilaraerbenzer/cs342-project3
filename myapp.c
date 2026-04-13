#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <signal.h>
 
#include "rsm.h"
 
#define NUMP 3
#define NUMR 5
 
static void setarray(int r[MAX_RT], int m, ...)
{
    va_list valist;
    va_start(valist, m);
    for (int i = 0; i < m; i++) {
        r[i] = va_arg(valist, int);
    }
    va_end(valist);
}
 
static void child_body(int apid, int avoid)
{
    int claim[MAX_RT] = {0};
    int req1[MAX_RT] = {0};
    int req2[MAX_RT] = {0};
    int rel[MAX_RT] = {0};
 
    if (rsm_process_started(apid) != 0) exit(2);
 
    if (avoid) {
        // Each process may need up to 2 resources (among R0,R1,R2).
        // The remaining types (R3,R4) are unused but satisfy ">=5 types".
        if (apid == 0) setarray(claim, NUMR, 1, 1, 0, 0, 0); // R0,R1
        if (apid == 1) setarray(claim, NUMR, 0, 1, 1, 0, 0); // R1,R2
        if (apid == 2) setarray(claim, NUMR, 1, 0, 1, 0, 0); // R2,R0
        if (rsm_claim(claim) != 0) exit(3);
    }
 
    // Phase 1: try to grab one distinct resource.
    if (apid == 0) setarray(req1, NUMR, 1, 0, 0, 0, 0); // take R0
    if (apid == 1) setarray(req1, NUMR, 0, 1, 0, 0, 0); // take R1
    if (apid == 2) setarray(req1, NUMR, 0, 0, 1, 0, 0); // take R2
    if (rsm_request(req1) != 0) exit(4);
 
    // Let others reach the same point.
    sleep(1);
 
    // Phase 2: request a second resource, forming a cycle if avoidance is off:
    // P0 holds R0, requests R1; P1 holds R1, requests R2; P2 holds R2, requests R0.
    if (apid == 0) setarray(req2, NUMR, 0, 1, 0, 0, 0);
    if (apid == 1) setarray(req2, NUMR, 0, 0, 1, 0, 0);
    if (apid == 2) setarray(req2, NUMR, 1, 0, 0, 0, 0);
 
    if (rsm_request(req2) != 0) exit(5);
 
    // If we get here, avoidance kept the system safe; release everything and exit.
    for (int j = 0; j < NUMR; j++) rel[j] = req1[j] + req2[j];
    rsm_release(rel);
    rsm_process_ended();
    exit(0);
}
 
int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: ./myapp avoidflag(0|1)\n");
        return 1;
    }
 
    int avoid = atoi(argv[1]) ? 1 : 0;
    int exist[NUMR] = {1, 1, 1, 1, 1};
 
    if (rsm_init(NUMP, NUMR, exist, avoid) != 0) {
        printf("rsm_init failed\n");
        return 2;
    }
 
    pid_t pids[NUMP];
    for (int i = 0; i < NUMP; i++) {
        pid_t n = fork();
        if (n == 0) {
            child_body(i, avoid);
        }
        pids[i] = n;
    }
 
    int detected = 0;
    for (int t = 0; t < 10; t++) {
        sleep(1);
        rsm_print_state("myapp: current state");
        int ret = rsm_detection();
        if (ret > 0) {
            detected = ret;
            printf("deadlock detected, count=%d\n", ret);
            rsm_print_state("myapp: state at deadlock");
            break;
        }
    }
 
    // In flag=0 mode, we expect a deadlock and children may block forever.
    // Kill them so we can cleanly exit and destroy shared memory.
    if (!avoid && detected > 0) {
        for (int i = 0; i < NUMP; i++) {
            if (pids[i] > 0) kill(pids[i], SIGKILL);
        }
    }
 
    for (int i = 0; i < NUMP; i++) {
        waitpid(pids[i], NULL, 0);
    }
 
    rsm_destroy();
    return 0;
}
