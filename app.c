#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "rsm.h"

#define NUMR 1        // number of resource types
#define NUMP 2        // number of processes

int AVOID = 1;
int exist[1] =  {8};  // resources existing in the system

void pr (int apid, char astr[], int m, int r[])
{
    int i;
    printf ("process %d, %s, [", apid, astr);
    for (i = 0; i < m; ++i) {
        if (i == (m-1))
            printf ("%d", r[i]);
        else
            printf ("%d,", r[i]);
    }
    printf ("]\n");
}


void setarray (int r[MAX_RT], int m, ...)
{
    va_list valist;
    int i;
    
    va_start (valist, m);
    for (i = 0; i < m; i++) {
        r[i] = va_arg(valist, int);
    }
    va_end(valist);
    return;
}


void func_p1 (int apid)
{
    int request1[MAX_RT];
    int request2[MAX_RT];
    int claim[MAX_RT];
    
    rsm_process_started (apid);

    setarray(claim, NUMR, 8);
    rsm_claim (claim);
    
    setarray(request1, NUMR, 5);
    pr (apid, "REQ", NUMR, request1);
    rsm_request (request1);

    sleep(4);

    setarray(request2, NUMR, 3);
    pr (apid, "REQ", NUMR, request2);
    rsm_request (request2);

    rsm_release (request1);
    rsm_release (request2);

    rsm_process_ended();
    exit(0);
}


void func_p2 (int apid)
{
    int request1[MAX_RT];
    int request2[MAX_RT];
    int claim[MAX_RT];

    rsm_process_started (apid);

    setarray(claim, NUMR, 8);
    rsm_claim (claim);

    setarray(request1, NUMR, 2);
    pr (apid, "REQ", NUMR, request1);
    rsm_request (request1);

    sleep(2);
    
    setarray(request2, NUMR, 4);
    pr (apid, "REQ", NUMR, request2);
    rsm_request (request2);

    rsm_release (request1);
    rsm_release (request2);

    rsm_process_ended ();
    exit(0);
}


int main(int argc, char **argv)
{
    int i;
    int count;
    int ret;
    int n;

    if (argc != 2) {
        printf ("usage: ./app avoidflag\n");
        exit (1);
    }

    AVOID = atoi (argv[1]);
    
    if (AVOID == 1)
        rsm_init (NUMP, NUMR, exist, 1);
    else
        rsm_init (NUMP, NUMR, exist, 0);

    i = 0;  // we select a pid for the child process
    n = fork();
    if (n == 0) {
        func_p1(i);
    }

    i = 1;  // we select a tid for the thread
    n = fork();
    if (n == 0) {
        func_p2(i);
    }

    count = 0;
    while ( count < 10) {
        sleep(1);
        rsm_print_state("The current state");
        ret = rsm_detection();
        if (ret > 0) {
            printf ("deadlock detected, count=%d\n", ret);
            rsm_print_state("state after deadlock");
        }
        count++;
    }
    
 
    for (i = 0; i < NUMP; ++i) {
            wait (NULL);
    }


   rsm_destroy();
}

