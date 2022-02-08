/*

References:
- Mutex lock: https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html

*/


#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

pthread_mutex_t count_mutex;
long long count;

#define CLOCK_TYPE CLOCK_REALTIME
#define ITERATIONS 5

void printExecutionTime(struct timespec * tpStart, struct timespec * tpEnd, char *metric);
void incrementCount();
void accessMainMemory();

int main(int argc, char **argv)
{
    

    // Mutex lock/unlock
    for (int i = 0; i < ITERATIONS; i++)
        incrementCount();
    
    // Main Memory reference
    for (int i = 0; i < ITERATIONS; i++)
        accessMainMemory();

    return 0;
}


void accessMainMemory()
{
    struct timespec start;
    struct timespec stop;
    char *ptr = "abc";
    char c;

    clock_gettime(CLOCK_TYPE, &start);
    c = *ptr; // deref - mem access
    clock_gettime(CLOCK_TYPE, &stop);

    printExecutionTime(&start, &stop, "Main Memory Access");
}

void printExecutionTime(struct timespec * tpStart, struct timespec * tpEnd, char *metric)
{
    long sec = tpEnd->tv_sec - tpStart->tv_sec;
    long nsec =  tpEnd->tv_nsec - tpStart->tv_nsec;
    
    printf("Execution time for %s\n", metric);
    printf("Seconds diff: %ld\n", sec);
    printf("Nano seconds diff: %ld\n", nsec);
}

void incrementCount()
{
    struct timespec startLock;
    struct timespec stopLock;
    struct timespec startUnlock;
    struct timespec stopUnlock;

    clock_gettime(CLOCK_TYPE, &startLock);
	pthread_mutex_lock(&count_mutex);
    clock_gettime(CLOCK_TYPE, &stopLock);

    count = count + 1;

    clock_gettime(CLOCK_TYPE, &startUnlock);
	pthread_mutex_unlock(&count_mutex);
    clock_gettime(CLOCK_TYPE, &stopUnlock);

    printExecutionTime(&startLock, &stopLock, "Mutex Lock");
    printExecutionTime(&startUnlock, &stopUnlock, "Mutex Unlock");
}


