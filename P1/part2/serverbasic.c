#include <stdio.h>
#include "udp.h"

#define BUFFER_SIZE (65000)
#define REPLY_SIZE (4)

// server code
int main(int argc, char *argv[]) {
    int sd = UDP_Open(atoi(argv[1]));
    int drop_count = atoi(argv[2]);
    char message[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    sprintf(reply, "ack");
    assert(sd > -1);
    while (1) {
        struct sockaddr_in addr;
        int random_number = rand() % 100;
        printf("Server will be dropping the packets %d times\n",drop_count);
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
        // printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
        if (random_number < drop_count){
            continue;
        }
        if (rc > 0) {
            rc = UDP_Write(sd, &addr, reply, REPLY_SIZE);
            printf("server:: replied\n");
        } 
    }
    return 0; 
}