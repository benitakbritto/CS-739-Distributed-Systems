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
#define FOUR_KB                 4096    

void printExecutionTime(struct timespec * tpStart, struct timespec * tpEnd, char *metric);
void incrementCount();
void accessMainMemory();
void performDiskSeek();
void readMemorySeq();
void readOneMBSeq(); 
void readSSDRandom();
void showFlagUsage();


int main(int argc, char **argv)
{
    srand(time(NULL));
    if (argc != 2)
    {
        printf("Failure: Specify argument\n");
        showFlagUsage();
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
                readOneMBSeq();
            break;
        }
        // UNSURE: Disk seek
        case 12:
        {
            for (int i = 0; i < ITERATIONS; i++)
                performDiskSeek();
            break;
        }
        // Read 1 MB sequentially from disk
        case 13:
        {
            for (int i = 0; i < ITERATIONS; i++)
                readOneMBSeq();
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
            showFlagUsage();
            exit(1);
        }
    }
    
    return 0;
}

void showFlagUsage()
{
    printf("Usage: -t <num>\n");
    printf("\t where num is:\n");
    printf("\t 1 = L1 cache reference\n");
    printf("\t 2 = Branch mispredict\n");
    printf("\t 3 = L2 cache reference\n");
    printf("\t 4 = Mutex lock/unlock\n");
    printf("\t 5 = Main memory reference\n");
    printf("\t 6 = Compress 1K bytes with Zippy\n");
    printf("\t 7 = Send 1K bytes over 1 Gbps network\n");
    printf("\t 8 = Read 4K randomly from SSD*\n");
    printf("\t 9 = Read 1 MB sequentially from memory\n");
    printf("\t 10 = Round trip within same datacenter\n");
    printf("\t 11 = Read 1 MB sequentially from SSD*\n");
    printf("\t 12 = Disk seek\n");
    printf("\t 13 = Read 1 MB sequentially from disk\n");
    printf("\t 14 = Send packet CA->Netherlands->CA\n");
}

void readSSDRandom()
{
    struct timespec start;
    struct timespec stop;
    char *fsimage = ONE_MB_FILE_NAME;
    int fd;
    unsigned char buf[FOUR_KB];
    int maxRandomNumber = 100024;

    if ((fd = open(fsimage, O_RDWR, 0666)) < 0)
    {
        dbgprintf("Failure: open() failed with errno: %d\n", errno);
        exit(1);
    }

    int seekLocation = rand() % maxRandomNumber;  
    printf("seekLocation: %d\n", seekLocation);

    clock_gettime(CLOCK_TYPE, &start);
    lseek(fd, seekLocation, SEEK_SET);    
    read(fd, &buf, FOUR_KB);

    clock_gettime(CLOCK_TYPE, &stop);

    printExecutionTime(&start, &stop, "Read 4K randomly from SSD");
    close(fd);
}

void readOneMBSeq()
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

    printExecutionTime(&start, &stop, "Read 1 MB sequentially");
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
    
    printf("Execution time for %s: %ld s %ld ns \n", metric, sec, nsec);
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


