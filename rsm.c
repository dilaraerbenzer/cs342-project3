#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
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

#define TRUE 1
#define FALSE 0

struct shared_data {
    int p_count;
    int r_count;
    int avoid;

    int ExistingV[MAX_RT];
    int AvailV[MAX_RT];
    int AllocationM[MAX_PR][MAX_RT];
    int RequestM[MAX_PR][MAX_RT];

    //  avoidance
    int MaxM[MAX_PR][MAX_RT];
    int NeedM[MAX_PR][MAX_RT];  
};

int shared_size = sizeof(struct shared_data);

int p_id;

void save_to_shared(struct shared_data d) {
    int shm_fd;
    void *ptr;
    shm_fd = shm_open(SHNAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { printf("shared memory failed\n"); exit(-1); }
    ftruncate(shm_fd, shared_size);

    ptr = mmap(0,shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) { printf("Map failed\n"); exit(-1); }
    ((struct shared_data*)ptr)[0] = d;
}

struct shared_data read_from_shared() {
    int shm_fd;
    void *ptr;
    shm_fd = shm_open(SHNAME, O_RDWR, 0666);
    if (shm_fd == -1) { printf("shared memory failed\n"); exit(-1); }
    ftruncate(shm_fd, shared_size);

    ptr = mmap(0,shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) { printf("Map failed\n"); exit(-1); }
    return ((struct shared_data*)ptr)[0];
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

    struct shared_data data = {p_count, r_count, avoid };
    memcpy(data.ExistingV, exist, r_count * sizeof(int));
    memcpy(data.AvailV, exist, r_count * sizeof(int));
    for (int i = 0; i < p_count; i++) {
        for (int j = 0; j < r_count; j++) {
            data.AllocationM[i][j] = 0;
            data.RequestM[i][j] = 0;
            data.MaxM[i][j] = 0;
            data.NeedM[i][j] = 0;
        }
    }

    save_to_shared(data);
    
    return  (ret);
}

int rsm_destroy()
{
    int ret = 0;
    
    return (ret);
}

int rsm_process_started(int apid)
{
    int ret = 0;
    p_id = apid;

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
    struct shared_data data = read_from_shared();
    for (int i = 0; i < data.r_count; i++) {
        data.AvailV[i] -= request[i];
        data.AllocationM[p_id][i] += request[i];
    }
    save_to_shared(data);
    
    return(ret);
}


int rsm_release (int release[])
{
    int ret = 0;
    struct shared_data data = read_from_shared();
    for (int i = 0; i < data.r_count; i++) {
        data.AvailV[i] += release[i];
        data.AllocationM[p_id][i] -= release[i];
    }
    save_to_shared(data);

    return (ret);
}


int rsm_detection()
{
    int ret = 0;
    
    return (ret);
}

void print_matrix (int rows, int cols, int data[][MAX_RT]) {
    printf("    ");
    for (int i = 0; i < cols; i++) {
        printf(" R%d", i);
    }
    printf("\n");
    for (int i = 0; i < rows; i++) {
        printf("P%d:  ", i);
        for (int j = 0; j < cols; j++) {
            printf("%-2d ", data[i][j]);
        }
        printf("\n");
    }
}

void print_vector (int cols, int data[cols]) {
    printf("    ");
    for (int i = 0; i < cols; i++) {
        printf(" R%d", i);
    }
    printf("\n");
    printf("     ");
    for (int j = 0; j < cols; j++) {
        printf("%-2d ", data[j]);
    }
    printf("\n");
}


void rsm_print_state (char hmsg[])
{
    printf("##########################\n");
    printf("%s\n",hmsg);
    printf("##########################\n");
    struct shared_data data = read_from_shared();
    printf("Exist:\n");
    print_vector(data.r_count, data.ExistingV);
    printf("Available:\n");
    print_vector(data.r_count, data.AvailV);
    printf("Allocation:\n");
    print_matrix(data.p_count, data.r_count, data.AllocationM);
    printf("Request:\n");
    print_matrix(data.p_count, data.r_count, data.RequestM);
    printf("MaxDemand:\n");
    print_matrix(data.p_count, data.r_count, data.MaxM);
    printf("Need:\n");
    print_matrix(data.p_count, data.r_count, data.NeedM);

    return;
}
