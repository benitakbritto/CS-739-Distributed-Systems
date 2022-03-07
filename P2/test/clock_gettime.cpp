/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

u_int64_t  total_time;
    struct timespec start, end;
int main()
{
 
    
    
 
    clock_gettime(CLOCK_MONOTONIC, &start);
   
    clock_gettime(CLOCK_MONOTONIC, &end);
 
   
   
 total_time =  end.tv_nsec - start.tv_nsec;
  printf("File read time = %lu nanoseconds \n", total_time);
    return 0;
    
    
    
}
