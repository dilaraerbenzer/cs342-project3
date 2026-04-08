
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "rsm.h"

int N; // number of processes
int M; // number of resource types

int ExistingV[MAX_RT];
int AvailV[MAX_RT];
int AllocationM[MAX_PR][MAX_RT];
int RequestM[MAX_PR][MAX_RT];

//  avoidance
int MaxM[MAX_PR][MAX_RT];
int NeedM[MAX_PR][MAX_RT];

#define TRUE 1
#define FALSE 0



//..... definitions/variables .....
//.....
//.....
int rsm_init(int p_count, int r_count, int exist[],  int avoid)
{
    int ret = 0;
    
    return  (ret);
}

int rsm_destroy()
{
    int ret = 0;
    
    return (ret);
}

int rsm_process_started(int pid)
{
    int ret = 0;
    return (ret);
}


int rsm_process_ended()
{
    int ret = 0;
    return (ret);
}


int rsm_claim (int claim[])
{
    int ret = 0;
    return(ret);
}

int rsm_request (int request[])
{
    int ret = 0;
    
    return(ret);
}


int rsm_release (int release[])
{
    int ret = 0;

    return (ret);
}


int rsm_detection()
{
    int ret = 0;
    
    return (ret);
}


void rsm_print_state (char hmsg[])
{
    return;
}
