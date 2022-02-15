#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define ITERATIONS              500
#define DEBUG                   0
#define dbgprintf(...)          if (DEBUG) { printf(__VA_ARGS__); }

enum clockType {REALTIME, MONOTONIC, PROCESS_CPUTIME_ID, THREAD_CPUTIME_ID} c;

void executeFunction(int clockType);
clockid_t getClockId(int clockType);
void simpleLoop(clockid_t clk_id);
void printExecutionTime(struct timespec * tpStart, struct timespec * tpEnd);
void printResolutionOfClockType(clockid_t clk_id);
void showUsage();

int main(int argc, char **argv)
{
    int clockType;

    if (argc != 2)
    {
        printf("Incorrect arguments\n");
        showUsage();
        exit(-1);
    }

    clockType = atoi(argv[1]);
    executeFunction(clockType);
    printResolutionOfClockType(clockType);
}

void showUsage()
{
    printf("Run using ./part0 <num>\n");
    printf("\t 0 = REALTIME \n");
    printf("\t 1 = MONOTONIC \n");
    printf("\t 2 = PROCESS_CPUTIME_ID \n");
    printf("\t 3 = THREAD_CPUTIME_ID \n");
}

void executeFunction(int clockType)
{
    clockid_t clk_id = getClockId(clockType);
    simpleLoop(clk_id);
}

void simpleLoop(clockid_t clk_id)
{
    int rc;
    struct timespec tpStart;
    struct timespec tpEnd;
    rc = clock_gettime(clk_id, &tpStart);
    if (rc == -1)
    {
        perror("Failed: clock_gettime\n");
    }

    for (int i = 0; i < ITERATIONS; i++)
    {
        printf("Iteration %d \n", i);
    }

    rc = clock_gettime(clk_id, &tpEnd);
    if (rc == -1)
    {
        perror("Failed: clock_gettime\n");
    }

    printExecutionTime(&tpStart, &tpEnd);
}

void printExecutionTime(struct timespec * tpStart, struct timespec * tpEnd)
{
    long sec = tpEnd->tv_sec - tpStart->tv_sec;
    long nsec =  tpEnd->tv_nsec - tpStart->tv_nsec;
    printf("Simple Loop Execution time: %ld s %ld ns\n", sec, nsec);
}

clockid_t getClockId(int clockType)
{
    clockid_t clk_id;
    switch (clockType)
    {
        case REALTIME:
            printf("REALTIME clock invoked\n");
            clk_id = CLOCK_REALTIME;
            break;
        case MONOTONIC:
            printf("MONOTONIC clock invoked\n");
            clk_id = CLOCK_MONOTONIC;
            break;
        case PROCESS_CPUTIME_ID:
            printf("PROCESS_CPUTIME_ID clock invoked\n");
            clk_id = CLOCK_PROCESS_CPUTIME_ID;
            break;
        case THREAD_CPUTIME_ID:
            printf("THREAD_CPUTIME_ID clock invoked\n");
            clk_id = CLOCK_THREAD_CPUTIME_ID;
            break;
        default:
            perror("Incorrect clock type\n");
            exit(-1);
            break;
    }

    return clk_id;
}

void printResolutionOfClockType(clockid_t clk_id)
{
    struct timespec tp;
    int rc;
    rc = clock_getres(clk_id, &tp);
    if (rc == -1)
    {
        perror("Failure: clock_getres()\n");
        exit(-1);
    }

    printf("Precision of clock: %ld s %ld ns\n", tp.tv_sec, tp.tv_nsec);
}
