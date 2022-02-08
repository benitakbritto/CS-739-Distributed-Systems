/*
References:
1) https://linux.die.net/man/3/clock_gettime
2) https://gist.github.com/pfigue/9ce8a2c0b14a2542acd7
3) https://users.pja.edu.pl/~jms/qnx/help/watcom/clibref/qnx/clock_gettime.html\\

Compile:
gcc -o part0 part0.c -Wall

*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define ITERATIONS 5

enum clockType {REALTIME, MONOTONIC, PROCESS_CPUTIME_ID, THREAD_CPUTIME_ID} c;

void executeFunction(int clockType);
clockid_t getClockId(int clockType);
void simpleLoop(clockid_t clk_id);
void printExecutionTime(struct timespec * tpStart, struct timespec * tpEnd);
void printResolutionOfClockType(clockid_t clk_id);

int main(int argc, char **argv)
{
    int clockType;

    if (argc != 2)
    {
        printf("Incorrect arguments\n");
        exit(-1);
    }

    clockType = atoi(argv[1]);
    executeFunction(clockType);
    printResolutionOfClockType(clockType);
}

void executeFunction(int clockType)
{
    clockid_t clk_id = getClockId(clockType);
    simpleLoop(clk_id);
    printResolutionOfClockType(clk_id);
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

    clock_gettime(clk_id, &tpEnd);
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
                                
    printf("Seconds diff: %ld\n", sec);
    printf("Nano seconds diff: %ld\n", nsec);
}

clockid_t getClockId(int clockType)
{
    clockid_t clk_id;
    switch (clockType)
    {
        case REALTIME:
            clk_id = CLOCK_REALTIME;
            break;
        case MONOTONIC:
            clk_id = CLOCK_MONOTONIC;
            break;
        case PROCESS_CPUTIME_ID:
            clk_id = CLOCK_PROCESS_CPUTIME_ID;
            break;
        case THREAD_CPUTIME_ID:
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

    printf("Precision: Seconds %ld \n", tp.tv_sec);
    printf("Precision: Nano Seconds %ld \n", tp.tv_nsec);
}
