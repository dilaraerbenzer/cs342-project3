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
#include <semaphore.h>
#include "rsm.h"

#define SHNAME "/kuromiena"

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
    
    sem_t mem_lock_sem;
    sem_t proc_sem[MAX_PR];
    int claim_count;
    sem_t barrier_sem;
};

int shared_size = sizeof(struct shared_data);

int p_id;
struct shared_data *ptr;

int init_shared_data() {
    int shm_fd;
    shm_fd = shm_open(SHNAME, O_RDWR, 0666);
    if (shm_fd == -1) { printf("shared memory failed\n"); return(-1); }

    ptr = mmap(0,shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) { printf("Map failed\n"); return(-1); }
    return 0;
}


int is_safe_state(void)
{
    int work[MAX_RT];
    int finish[MAX_PR];
    int i;
    int j;
 
    for (j = 0; j < ptr->r_count; j++) {
      work[j] = ptr->AvailV[j];
    }
    for (i = 0; i < ptr->p_count; i++) {
      finish[i] = 0;
    }
 
    int progress = 1;
    while (progress) {
        progress = 0;
        for (i = 0; i < ptr->p_count; i++) {
            if (finish[i]) continue;
            int can_finish = 1;
            for (j = 0; j < ptr->r_count; j++) {
                if (ptr->NeedM[i][j] > work[j]) { can_finish = 0; break; }
            }
            if (can_finish) {
                for (j = 0; j < ptr->r_count; j++)
                    work[j] += ptr->AllocationM[i][j];
                finish[i] = 1;
                progress = 1;
            }
        }
    }
    for (i = 0; i < ptr->p_count; i++)
        if (!finish[i]) {
          return 0;
        }
    return 1;
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

    data.claim_count = 0;

    int shm_fd = shm_open(SHNAME, O_RDWR | O_CREAT, 0666);
    if (shm_fd == -1) { printf("shared memory failed\n"); return(-1); }
    ftruncate(shm_fd, shared_size);

    ptr = mmap(0,shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) { printf("Map failed\n"); return(-1); }

    *ptr = data;

    sem_init(&ptr->mem_lock_sem, 1, 1);
    sem_init(&ptr->barrier_sem, 1, 0);

    for (int i = 0; i < p_count; i++) {
        sem_init(&ptr->proc_sem[i], 1, 0);
    }

    if (!avoid) {
        sem_post(&ptr->barrier_sem);
    }

    return  (ret);
}

int rsm_destroy()
{
    int ret = 0;
    
    sem_destroy(&ptr->mem_lock_sem);
    sem_destroy(&ptr->barrier_sem);
    
    for (int i = 0; i < ptr->p_count; i++) {
        sem_destroy(&ptr->proc_sem[i]);
    }
    
    munmap(ptr, shared_size);
    shm_unlink(SHNAME);
    
    return (ret);
}

int rsm_process_started(int apid)
{
    p_id = apid;
    return init_shared_data();
}


int rsm_process_ended()
{
  int ret = 0;
  sem_wait(&ptr->mem_lock_sem);

  for (int j = 0; j < ptr->r_count; j++) {
    ptr->AvailV[j] += ptr->AllocationM[p_id][j];
    ptr->AllocationM[p_id][j] = 0;
    ptr->RequestM[p_id][j] = 0;
    if (ptr->avoid) ptr->NeedM[p_id][j] = 0;
  }

  for (int i = 0; i < ptr->p_count; i++) {
    int waiting = 0;
    for (int j = 0; j < ptr->r_count; j++) {
      if (ptr->RequestM[i][j] > 0) { waiting = 1; break; }
    }
    if (waiting) sem_post(&ptr->proc_sem[i]);
  }

  sem_post(&ptr->mem_lock_sem);
  return (ret);
}


int rsm_claim (int claim[])
{
    if (!ptr->avoid) return 0;   
 
    sem_wait(&ptr->mem_lock_sem);
 
    for (int j = 0; j < ptr->r_count; j++) {
        if (claim[j] > ptr->ExistingV[j]) {
            sem_post(&ptr->mem_lock_sem);
            return -1;
        }
    }
 
    for (int j = 0; j < ptr->r_count; j++) {
        ptr->MaxM[p_id][j]  = claim[j];
        ptr->NeedM[p_id][j] = claim[j] - ptr->AllocationM[p_id][j];
    }
 
    ptr->claim_count++;
    if (ptr->claim_count == ptr->p_count) {
        sem_post(&ptr->barrier_sem);
    }
 
    sem_post(&ptr->mem_lock_sem);
    return 0;
}



int rsm_release (int release[])
{
    int ret = 0;
 
    sem_wait(&ptr->mem_lock_sem);
 
    for (int j = 0; j < ptr->r_count; j++) {
        if (release[j] > ptr->AllocationM[p_id][j]) {
            sem_post(&ptr->mem_lock_sem);
            return -1;
        }
    }
 
    for (int j = 0; j < ptr->r_count; j++) {
        ptr->AvailV[j] += release[j];
        ptr->AllocationM[p_id][j] -= release[j];
        if (ptr->avoid) {
          ptr->NeedM[p_id][j] += release[j];
        }
    }
 
    for (int i = 0; i < ptr->p_count; i++) {
        int waiting = 0;
        for (int j = 0; j < ptr->r_count; j++) {
            if (ptr->RequestM[i][j] > 0) {
              waiting = 1;
              break;
            }
        }
        if (waiting) {
          sem_post(&ptr->proc_sem[i]);
        }
    }
 
    sem_post(&ptr->mem_lock_sem);
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
    
    sem_wait(&ptr->mem_lock_sem);
    struct shared_data data = *ptr;
    
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

    sem_post(&ptr->mem_lock_sem);
    
    return;
}
