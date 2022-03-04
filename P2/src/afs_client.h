// grpc code here

#include <grpcpp/grpcpp.h>
#include <string>
#include <chrono>
#include <ctime>
#include "filesystemcomm.grpc.pb.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>

// Macros
#define DEBUG                 1
#define dbgprintf(...)        if (DEBUG) { printf(__VA_ARGS__); }
//#define SERVER_ADDR         "20.69.154.6:50051"
#define SERVER_ADDR           "0.0.0.0:50051"
#define MAX_RETRY             5
#define RETRY_TIME_START      1 // seconds
#define RETRY_TIME_MULTIPLIER 2


// Namespaces
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using filesystemcomm::FileSystemService;
using filesystemcomm::FetchRequest;
using filesystemcomm::FetchResponse;
using filesystemcomm::GetFileStatRequest;
using filesystemcomm::GetFileStatResponse;
using filesystemcomm::ListDirRequest;
using filesystemcomm::ListDirResponse;
using filesystemcomm::MakeDirRequest;
using filesystemcomm::MakeDirResponse;
using filesystemcomm::RemoveDirRequest;
using filesystemcomm::RemoveDirResponse;
using filesystemcomm::RemoveRequest;
using filesystemcomm::RemoveResponse;
using filesystemcomm::RenameRequest;
using filesystemcomm::RenameResponse;
using filesystemcomm::StoreRequest;
using filesystemcomm::StoreResponse;
using filesystemcomm::TestAuthRequest;
using filesystemcomm::TestAuthResponse;
using filesystemcomm::DirectoryEntry;
using filesystemcomm::Timestamp;
using filesystemcomm::FileStat;
using grpc::Status;
using grpc::StatusCode;
using namespace std;

namespace FileSystemClient
{
    class ClientImplementation 
    {
        public:
            ClientImplementation(std::shared_ptr<Channel> channel)
                : stub_(FileSystemService::NewStub(channel)) {}
        
            int MakeDir(std::string path, mode_t mode) 
            {
                dbgprintf("MakeDir: Entering function\n");
                MakeDirRequest request;
                MakeDirResponse reply;
                Status status;
                uint32_t retryCount = 0;
                
                request.set_pathname(path);
                request.set_mode(mode);

                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("MakeDir: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->MakeDir(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("MakeDir: RPC Success\n");
                    dbgprintf("MakeDir: Exiting function\n");
                    return 0;
                }
                else
                {
                    dbgprintf("MakeDir: RPC failure\n");
                    dbgprintf("MakeDir: Exiting function\n");
                    return -1;
                }
            }

            int GetFileStat(std::string path, struct stat *stbuf) 
            {
                dbgprintf("GetFileStat: Entering function\n");
                GetFileStatRequest request;
                GetFileStatResponse reply;
                Status status;
                uint32_t retryCount = 0;

                request.set_pathname(path);

                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("GetFileStat: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->GetFileStat(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("GetFileStat: RPC Success\n");
                    stbuf->st_ino = reply.status().ino();
                    stbuf->st_mode = reply.status().mode();
                    stbuf->st_nlink = reply.status().nlink();
                    stbuf->st_uid = reply.status().uid();
                    stbuf->st_gid = reply.status().gid();
                    stbuf->st_size = reply.status().size();
                    stbuf->st_blksize = reply.status().blksize();
                    stbuf->st_blocks = reply.status().blocks();
                    stbuf->st_atime = reply.status().atime();
                    stbuf->st_mtime = reply.status().mtime();
                    stbuf->st_ctime = reply.status().ctime();
                    dbgprintf("GetFileStat: Exiting function\n");
                    return 0;
                } 
                else 
                {
                    dbgprintf("GetFileStat: RPC Failed\n");
                    dbgprintf("GetFileStat: Exiting function\n");
                    return -1;
                }
            }
        
        private:
                std::unique_ptr<FileSystemService::Stub> stub_;
 
    };
}

