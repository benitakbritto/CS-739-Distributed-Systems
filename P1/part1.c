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
#define ONE_MB_FILE_NAME        "files/one_mb_file"
#define ONE_MB                  1000000
#define CHUNK_SIZE              1024     
#define THREE_KB                3072    

void printExecutionTime(struct timespec * tpStart, struct timespec * tpEnd, char *metric);
void incrementCount();
void accessMainMemory();
void performDiskSeek(); // UNSURE: Won't this be an SSD seek?
void readMemorySeq();
void readSSDSeq(); 
void readSSDRandom();


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Failure: Specify argument\n");
        exit(1);
    }

    switch(atoi(argv[1]))
    {
        // TODO: L1 cache reference  
        case 1:
        {

            break;
        }
        // TODO: Branch mispredict
        case 2:
        {

            break;
        }
        // TODO: L2 cache reference
        case 3:
        {

            break;
        }
        // Mutex lock/unlock
        case 4:
        {
            for (int i = 0; i < ITERATIONS; i++)
                incrementCount();
            break;
        }
        // Main memory reference
        case 5:
        {
            for (int i = 0; i < ITERATIONS; i++)
                accessMainMemory();
            break;
        }
        // TODO: Compress 1K bytes with Zippy 
        case 6:
        {

            break;
        }
        // TODO: Send 1K bytes over 1 Gbps network
        case 7:
        {

            break;
        }
        // TODO: Read 4K randomly from SSD*
        case 8:
        {
            for (int i = 0; i < ITERATIONS; i++)
                readSSDRandom();

            break;
        }
        // UNSURE: Read 1 MB sequentially from memory
        case 9:
        {
            for (int i = 0; i < ITERATIONS; i++)
                readMemorySeq();
            break;
        }
        // TODO: Round trip within same datacenter
        case 10:
        {

            break;
        }       
        // Read 1 MB sequentially from SSD*
        case 11:
        {
            for (int i = 0; i < ITERATIONS; i++)
                readSSDSeq();
            break;
        }
        // UNSURE: Disk seek
        case 12:
        {
            for (int i = 0; i < ITERATIONS; i++)
                performDiskSeek();
            break;
        }
        // TODO: Read 1 MB sequentially from disk
        case 13:
        {

            break;
        }
        // TODO: Send packet CA->Netherlands->CA
        case 14:
        {

            break;
        }
        default:
        {
            printf("Failure: Incorrect argument\n");
            exit(1);
        }
    }
    
    return 0;
}

void readSSDRandom()
{
    struct timespec start;
    struct timespec stop;
    char *fsimage = ONE_MB_FILE_NAME;
    int fd;
    unsigned char buf[CHUNK_SIZE];

    if ((fd = open(fsimage, O_RDWR, 0666)) < 0)
    {
        dbgprintf("Failure: open() failed with errno: %d\n", errno);
        exit(1);
    }

    // reads from 4kb .. 0 (backwards)
    int seekLocation = THREE_KB;
    clock_gettime(CLOCK_TYPE, &start);
    for (int i = seekLocation; i > 0; i -= CHUNK_SIZE)
    {
        dbgprintf("seek loc %d\n", i);
        lseek(fd, i, SEEK_SET);
        read(fd, &buf, CHUNK_SIZE);
    }
    clock_gettime(CLOCK_TYPE, &stop);
    
    printExecutionTime(&start, &stop, "Read 4K randomly from SSD");
    close(fd);
}

void readSSDSeq()
{
    struct timespec start;
    struct timespec stop;
    char *fsimage = ONE_MB_FILE_NAME;
    int fd;
    unsigned char buf[ONE_MB];

    if ((fd = open(fsimage, O_RDWR, 0666)) < 0)
    {
        dbgprintf("Failure: open() failed with errno: %d\n", errno);
        exit(1);
    }

    clock_gettime(CLOCK_TYPE, &start);
    read(fd, &buf, ONE_MB);
    clock_gettime(CLOCK_TYPE, &stop);

    printExecutionTime(&start, &stop, "Read 1 MB sequentially from SSD");
    close(fd);
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


