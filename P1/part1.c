/*

References:
- Mutex lock: https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html

*/


#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t count_mutex;
long long count;

#define DEBUG                   1
#define dbgprintf(...)          if (DEBUG) { printf(__VA_ARGS__); }
#define CLOCK_TYPE              CLOCK_REALTIME
#define ITERATIONS              5
#define SEEK_LOCATION           900
#define FILE_NAME               "README.md"

void printExecutionTime(struct timespec * tpStart, struct timespec * tpEnd, char *metric);
void incrementCount();
void accessMainMemory();
void performDiskSeek();
void readMemorySeq();

int main(int argc, char **argv)
{
    // Mutex lock/unlock
    for (int i = 0; i < ITERATIONS; i++)
        incrementCount();
    
    // Main Memory reference
    for (int i = 0; i < ITERATIONS; i++)
        accessMainMemory();

    // Disk Seek
    for (int i = 0; i < ITERATIONS; i++)
        performDiskSeek();

    // Read 1 MB sequentially from memory
    for (int i = 0; i < ITERATIONS; i++)
        readMemorySeq();

    return 0;
}

void readMemorySeq()
{
    long long size = 125000;
    dbgprintf("long long size: %ld\n", sizeof(long long));
    long long * ptr = calloc(size, 8);
    long long c;
    struct timespec start;
    struct timespec stop;

    clock_gettime(CLOCK_TYPE, &start);
    for (int i = 0; i < size; i++)
    {
        c = *ptr;
        ptr++;
    }
    clock_gettime(CLOCK_TYPE, &stop);

    printExecutionTime(&start, &stop, "Read 1 MB sequentially from memory");
}


void performDiskSeek()
{
    struct timespec start;
    struct timespec stop;
    char *fsimage = FILE_NAME;
    int fd;

    if ((fd = open(fsimage, O_RDWR, 0666)) < 0)
    {
        dbgprintf("Failure: open() failed with errno: %d\n", errno);
        exit(1);
    }

    clock_gettime(CLOCK_TYPE, &start);
    lseek(fd, SEEK_LOCATION, SEEK_SET);
    clock_gettime(CLOCK_TYPE, &stop);

    printExecutionTime(&start, &stop, "Disk Seek");
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


