#include <limits.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "afs_client.h"

#define SERVER_ADDR  "0.0.0.0:50051"

using namespace std;
using namespace FileSystemClient;

ClientImplementation * client;

int main(int argc, char* argv[]) 
{
    std::string target_address(SERVER_ADDR);
    
    
    client = new ClientImplementation(grpc::CreateChannel(target_address,
                                grpc::InsecureChannelCredentials()));

    client->MakeDir("try", S_IRWXU);

    return 0;
}