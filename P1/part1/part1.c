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
#define ITERATIONS              100
#define SEEK_LOCATION           900
#define FILE_NAME               "files/one_mb_file"
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
void calculateL1Cache();
void branchMispredict();
void calculateL2Cache();

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
        // L1 cache reference  
        case 1:
        {

            calculateL1Cache();
            break;
        }
        // Branch mispredict
        case 2:
        {
            branchMispredict();
            break;
        }
        // L2 cache reference
        case 3:
        {
            calculateL2Cache();
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
            accessMainMemory();
            break;
        }
        // Compress 1K bytes with Zippy 
        case 6:
        {
	    printf("Check README.md for directions\n");
            break;
        }
        // Send 1K bytes over 1 Gbps network
        case 7:
        {
	    printf("Check README.md. Executed using a command\n");
            break;
        }
        // Read 4K randomly from SSD*
        case 8:
        {
            for (int i = 0; i < ITERATIONS; i++)
                readSSDRandom();

            break;
        }
        // Read 1 MB sequentially from memory
        case 9:
        {
            readMemorySeq();
            break;
        }
        // Round trip within same datacenter
        case 10:
        {
	    printf("Check README.md. Executed using a command\n");
            break;
        }       
        // Read 1 MB sequentially from SSD*
        case 11:
        {
            for (int i = 0; i < ITERATIONS; i++)
                readOneMBSeq();
            break;
        }
        // Disk seek
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
        // Send packet CA->Netherlands->CA
        case 14:
        {
            printf("Check README.md. Executed using a command\n");
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
    printf("Usage: ./part1 <num>\n");
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

    if ((fd = open(fsimage, O_RDWR, 0666)) < 0)
    {
        dbgprintf("Failure: open() failed with errno: %d\n", errno);
        exit(1);
    }

    int seekLocation = rand();
    printf("seekLocation: %d\n", seekLocation);

    clock_gettime(CLOCK_TYPE, &start);
    lseek(fd, seekLocation, SEEK_SET);    
    read(fd, &buf, FOUR_KB);

    clock_gettime(CLOCK_TYPE, &stop);

    printExecutionTime(&start, &stop, "Read 4K randomly from SSD");
    close(fd);
}

void calculateL1Cache()
{
	struct timespec start;
	struct timespec stop;
	int arraySize = 30000; // L1 size is 128 Kib
	int loops = 10000;
        int *array = (int *)malloc(sizeof(int) * arraySize);
        // initialize array
	for(int i=0; i<arraySize; i++)
	  array[i] = i+1;
        int x=0;
	// calculate base time
	clock_gettime(CLOCK_TYPE, &start);
	for(int i=0; i<loops; i++){
          x=0;
	  for(int j=0; j<arraySize; j++){
            x++;
	  }
	}
        clock_gettime(CLOCK_TYPE, &stop);
        long long int baseNsecond = (stop.tv_sec - start.tv_sec) * 1000000000L +                                       (stop.tv_nsec - start.tv_nsec);
        printf("base time %d\n",baseNsecond);

	// bring the array into L1 cache
	for(int i=0; i<loops; i++){
          for(int j=0; j<arraySize; j++){
            x = array[j];
	  }
	}
        int var;
        //calculate L1 access + loop time
	clock_gettime(CLOCK_TYPE, &start);
	for(int i=0; i<loops; i++){
          x=0;
	  for(int j=0; j<arraySize; j++){
            x++;
	    var = array[j];
	  }
	}
        clock_gettime(CLOCK_TYPE, &stop);
        long long int finalNsecond = (stop.tv_sec - start.tv_sec) * 1000000000L                                         + (stop.tv_nsec - start.tv_nsec);
        printf("final time %d\n",finalNsecond);
	printf("time for L1 Cache %f nsec\n", ((finalNsecond - baseNsecond)*1.0)/
				            (loops*arraySize*1.0));

}

void calculateL2Cache()
{
	struct timespec start;
	struct timespec stop;
	int arraySize = 250000;   // L2 size on CSL is 1 Mib = 250,000 integers
	int l1Size = 34000;       // size more than L1 size ie 128Kib = 32000 integers
	int loops = 1000;        // number of iterations
        int *array = (int *)malloc(sizeof(int) * arraySize);
        for(int i=0; i<arraySize; i++)
	  array[i] = i+1;
        int x=0;
        int itr;
	clock_gettime(CLOCK_TYPE, &start);
	for(int i=0; i<loops; i++){
	  x=0;
	  for(int j=0; j<arraySize; j+=16){      // increment by 16 to avoid cache line prefetch
            x++;
	  }
	}
        clock_gettime(CLOCK_TYPE, &stop);
        long long int baseNsecond = (stop.tv_sec - start.tv_sec) * 1000000000L + 
		                     (stop.tv_nsec - start.tv_nsec);
        printf("base time %d\n",baseNsecond);

	// bring the array into L2 cache
	for(int i=0; i<loops; i++){
          for(int j=0; j<arraySize; j++){
            x = array[j];
	  }
	}
        int var;
        x = 0;

	clock_gettime(CLOCK_TYPE, &start);
	for(int i=0; i<loops; i++){
	  x=0;
	  for(int j=0; j<arraySize; j+=16){
	    x++;
            var = array[j];
	  }
	}
        clock_gettime(CLOCK_TYPE, &stop);
        long long int finalNsecond = (stop.tv_sec - start.tv_sec) * 1000000000L + 
		                     (stop.tv_nsec - start.tv_nsec);
        printf("final time %d\n",finalNsecond);
	printf("time for L2 Cache %f \n", ((finalNsecond - baseNsecond)*16.0)/
				            (loops*1.0*arraySize));

}

static inline void timespec_diff(struct timespec *a, struct timespec *b,
  struct timespec *result) {
  result->tv_sec  = a->tv_sec  - b->tv_sec;
  result->tv_nsec = a->tv_nsec - b->tv_nsec;
  if (result->tv_nsec < 0) {
      --result->tv_sec;
      result->tv_nsec += 1000000000L;
  }
}

double timespec_to_double(struct timespec *a){
  return a->tv_sec*1e6 + a->tv_nsec/1000.0;
}

void branchMispredict()
{
    struct timespec ts1, ts2, ts3, ts5;
    int j = 0;
    // measuring correct branch prediction times
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    for(int i = 0; i<1000000;i++){
        if(j==i){
            j++;
        } else{
            j--;
        }
    }

    // measuring 50% correct prediction +50% misprediction time
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    for(int i = 0; i < 1000000; i++){
        if(rand()%1000000 <500000){
            j++;
        }else{
            j--;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &ts3);

    timespec_diff(&ts2, &ts1, &ts5);
    long int a = ts5.tv_nsec;
    timespec_diff(&ts3, &ts2, &ts5);
    long int b = ts5.tv_nsec;
    // subtracting both to get only misprediction time and averaging
    printf("time for branch mispredict %ld\n ns", (b-a)/500000);
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
    struct timespec start;
    struct timespec stop;
    long FOUR_HUNDRED_MB = 400000000;
    long EIGHT_MB = 8000000;
    
    // create an array that is bigger than all caches combined
    char *ptr = calloc(FOUR_HUNDRED_MB, sizeof(char));
    char c;

    for (int i = 0; i < ITERATIONS; i++)
    {
        long index = (i * ONE_MB + EIGHT_MB) % FOUR_HUNDRED_MB;
        clock_gettime(CLOCK_TYPE, &start);
        for (int j = 0; j < ONE_MB; j++)
        {
            long k = (index + j) % FOUR_HUNDRED_MB;
        }
        clock_gettime(CLOCK_TYPE, &stop);
        printExecutionTime(&start, &stop, "offset");
    }

    for (int i = 0; i < ITERATIONS; i++)
    {
        long index = (i * ONE_MB + EIGHT_MB) % FOUR_HUNDRED_MB; // stride
        clock_gettime(CLOCK_TYPE, &start);
        for (int j = 0; j < ONE_MB; j++)
        {
            c = *(ptr + (index + j) % FOUR_HUNDRED_MB); // deref - mem access
        }
        clock_gettime(CLOCK_TYPE, &stop);
        printExecutionTime(&start, &stop, "Read 1 MB sequentially from memory");
    }
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

    int seekLocation = rand() % CHUNK_SIZE;
    clock_gettime(CLOCK_TYPE, &start);
    lseek(fd, seekLocation, SEEK_SET);
    clock_gettime(CLOCK_TYPE, &stop);

    printExecutionTime(&start, &stop, "Disk Seek");
}

void accessMainMemory()
{
    struct timespec start;
    struct timespec stop;
    long FOUR_HUNDRED_MB = 400000000;
    long EIGHT_MB = 8000000;
    
    // create an array that is bigger than all caches combined
    char *ptr = calloc(FOUR_HUNDRED_MB, sizeof(char));
    char c;

    for (int i = 0; i < ITERATIONS; i++)
    {
        long index = (i * ONE_MB + EIGHT_MB) % FOUR_HUNDRED_MB; // stride
        printf("%ld\n", index);
        clock_gettime(CLOCK_TYPE, &start);
        c = *(ptr + index); // deref - mem access
        clock_gettime(CLOCK_TYPE, &stop);
        printExecutionTime(&start, &stop, "Main Memory Access");
    }
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
