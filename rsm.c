#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "rsm.h"

#define SHNAME "kuromiena"

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

int shared_size = sizeof(int);

void save_to_shared(int i) {
    int shm_fd;
    void *ptr;
    shm_fd = shm_open(SHNAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { printf("shared memory failed\n"); exit(-1); }
    ftruncate(shm_fd, shared_size);

    ptr = mmap(0,shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) { printf("Map failed\n"); return -1; }
    ((int*)ptr)[0] = i;
}

void read_from_shared(int* i) {
    int shm_fd;
    void *ptr;
    shm_fd = shm_open(SHNAME, O_RDWR, 0666);
    if (shm_fd == -1) { printf("shared memory failed\n"); exit(-1); }
    ftruncate(shm_fd, shared_size);

    ptr = mmap(0,shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) { printf("Map failed\n"); return -1; }
    *i = ((int*)ptr)[0];
}

//..... definitions/variables .....
//.....
//.....
int rsm_init(int p_count, int r_count, int exist[],  int avoid)
{
    int ret = 0;
    if (p_count > MAX_PR || r_count > MAX_RT) {
        return -1;
    }

    save_to_shared(20);
    
    return  (ret);
}

int rsm_destroy()
{
    int ret = 0;
    
    return (ret);
}

int rsm_process_started(int apid)
{
    int i;
    read_from_shared(&i);
    save_to_shared(i+1);
    return i;

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
