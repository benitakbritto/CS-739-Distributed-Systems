// compile using 
// run using client ${client_port} ${server_hostname} ${server_port} 

#include <stdio.h>
#include <stdlib.h>
#include "udp.h"
#include <math.h>
#include <time.h>

/**
 * @fn timespec_diff(struct timespec *, struct timespec *, struct timespec *)
 * @brief Compute the diff of two timespecs, that is a - b = result.
 * @param a the minuend
 * @param b the subtrahend
 * @param result a - b
 */
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

enum Status{
  SUCCESS,
  FAILURE,
  TIMEOUT
};

struct Element{
  enum Status status;
  int bytes;
  struct timespec overhead;
  struct timespec rtt;
};

struct timespec ts1, ts2, ts5;

struct Element* send_packet(int *sd, struct sockaddr_in *addrSnd,struct sockaddr_in *addrRcv, char* message, int buffer_length){
  
    struct Element* ele = (struct Element*)malloc(sizeof(struct Element));
    
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    int rc = UDP_Write(*sd, addrSnd, message, buffer_length);

    clock_gettime(CLOCK_MONOTONIC, &ts2);

    if (rc < 0) {
      printf("client:: failed to send %d\n", errno);
      exit(1);
    }

    rc = UDP_Read(*sd, addrRcv, message, buffer_length);

    clock_gettime(CLOCK_MONOTONIC, &ts5);

    if (rc < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK){
        ele->status = TIMEOUT;
        timespec_diff(&ts2, &ts1, &ele->overhead);
        timespec_diff(&ts5, &ts1, &ele->rtt);
      } else {
        ele->status = FAILURE; 
      }
      ele->bytes = 0;	      
    } else{
      ele->status = SUCCESS;
      ele->bytes = rc;
      timespec_diff(&ts2, &ts1, &ele->overhead);
      timespec_diff(&ts5, &ts1, &ele->rtt);
    }   
    return ele;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in addrSnd, addrRcv;
    int client_port = atoi(argv[1]);
    char* server_host = argv[2];
    int server_port = atoi(argv[3]);
    struct timeval timeout;
    int sd = UDP_Open(client_port);
    int warmup_rounds = 10;
    int main_rounds = 1000;
    // int buffer_length[] = {4, 8, 512, 1024, 1400, 1600, 2048, 4096, 8192, 16384, 32768, 45000, 60000, 65000};
    // int buffer_length[] = {100, 500, 600,1500, 3000, 10000, 1000, 30000, 50000, 65000};
    int buffer_length[] = {65000};
    double times_local_rtt[main_rounds];
    double times_global_rtt[sizeof(buffer_length)/sizeof(int)];
    double times_local_oh[main_rounds];
    double times_global_oh[sizeof(buffer_length)/sizeof(int)];
    double rtt_time, overhead_time;

    FILE *fpt;
    fpt = fopen("readings20.csv", "w+");
    // fprintf(fpt,"size, rtt, overhead, bandwidth\n");
    fprintf(fpt,"iteration_num, rtt\n");

    UDP_FillSockAddr(&addrSnd, server_host, server_port);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt (sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0){
      printf("setting socket options failed %d\n", errno);
      exit(1);
    }

    for (int len = 0; len < sizeof(buffer_length)/sizeof(int); len++){
      printf("reading for buffer length %d\n", buffer_length[len]);
      sleep(1);
      int ack = 0, dropped = 0;
      int itr = 0;
      char message[buffer_length[len]];

      for (int i = 0;i < buffer_length[len] -1; i++){
        message[i] = 'a';
      }
      message[buffer_length[len] - 1] = '\0';

      for (int i = 0; i < warmup_rounds; i++){
        send_packet(&sd, &addrSnd, &addrRcv, message, buffer_length[len]);
      }

      while (ack != main_rounds) {
        struct Element* element = send_packet(&sd, &addrSnd, &addrRcv, message, buffer_length[len]);
        if (element->status == SUCCESS){
          ack++;
          rtt_time = timespec_to_double(&element->rtt);
          overhead_time = timespec_to_double(&element->overhead);
          if (ack < 10) printf("client:: got reply [size:%d contents:(%s), %0.6f, %0.6f\n", element->bytes, message, rtt_time, overhead_time);
        } else if(element->status == FAILURE){
          printf("Failure sending packet\n");
          continue;
        } else {
          printf("timeout for packet num %d\n", ack);
          dropped++;
          rtt_time = timespec_to_double(&element->rtt);
          overhead_time = timespec_to_double(&element->overhead);
          while (element->status != SUCCESS){
            dropped++;
            element = send_packet(&sd, &addrSnd, &addrRcv, message, buffer_length[len]);
            rtt_time += timespec_to_double(&element->rtt);
          }
          printf("client:: got reply [size:%d contents:(%s), %0.6f, %0.6f\n", element->bytes, message, rtt_time, overhead_time);
          ack++;
        } 
        times_local_rtt[itr] = rtt_time;
        times_local_oh[itr] = overhead_time;
        fprintf(fpt, "%d, %0.6f\n", itr, times_local_rtt[itr]);
        itr++;
      }

      long double total_time_rtt = 0;
      long double total_time_oh = 0;
      
      printf("Out of %d tried messages, %d are dropped and %d are success\n", ack + dropped, dropped, ack);
      for (int i = 0; i < main_rounds; i++){
        total_time_rtt += times_local_rtt[i];
        total_time_oh += times_local_oh[i];
        // printf("%.6f \n", times_local[i]);
      }
      times_global_rtt[len] = total_time_rtt/(main_rounds*1.0);
      times_global_oh[len] = total_time_oh/(main_rounds*1.0);
    }

    
    for(int i = 0; i < sizeof(buffer_length)/sizeof(int); i++){
      printf("%d \t %0.6f \t %0.6f \t %0.6f \n", buffer_length[i], times_global_rtt[i], times_global_oh[i], buffer_length[i]/times_global_rtt[i]);
      // fprintf(fpt,"%d, %0.6f, %0.6f, %0.6f\n", buffer_length[i], times_global_rtt[i], times_global_oh[i], buffer_length[i]/times_global_rtt[i]);
    }

    fclose(fpt);
    return 0;
}