#include<stdio.h>
#include<stdlib.h>
#include<time.h>

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

int main(int argc, char *argv[]) {
    struct timespec ts1, ts2, ts3, ts5;
    int j = 0;
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    for(int i = 0; i<1000000;i++){
        if(j==i){
            j++;
        } else{
            j--;
        }
    }

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
    // printf("%ld\n", a);
    timespec_diff(&ts3, &ts2, &ts5);
    long int b = ts5.tv_nsec;
    // printf("%ld\n", b);
    printf("%ld\n", (b-a)/500000);
}
