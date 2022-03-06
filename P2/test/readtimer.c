/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

u_int64_t base_time, total_time;
    struct timespec start, end;
int main()
{
 
    
    FILE* demo;
    int display;
 
    demo = fopen("text.txt", "r");
 
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (1) {
       
        display = fgetc(demo);
 
       
        if (feof(demo))
            break;
 
        
        
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
 
   
    fclose(demo);
 base_time =  end.tv_nsec - start.tv_nsec;
  printf("File read time = %lu nanoseconds \n", base_time);
    return 0;
}
