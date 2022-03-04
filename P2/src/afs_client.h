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

            
            int RemoveDir(std::string path) 
            {
                dbgprintf("RemoveDir: Entering function\n");
                RemoveDirRequest request;
                RemoveDirResponse reply;
                Status status;
                uint32_t retryCount = 0;

                request.set_pathname(path);

                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("RemoveDir: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->RemoveDir(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("RemoveDir: RPC Success\n");
                    dbgprintf("RemoveDir: Exiting function\n");
                    return 0;

                } 
                else 
                {
                    dbgprintf("RemoveDir: RPC Failure\n");
                    dbgprintf("RemoveDir: Exiting function\n");
                    return -1;
                }
            }

            int ReadDir(std::string path, void *buf, fuse_fill_dir_t filler) 
            {
                dbgprintf("ListDir: Entering function\n");
                ListDirRequest request;
                ListDirResponse reply;
                Status status;
                uint32_t retryCount = 0;

                request.set_pathname(path);

                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("ListDir: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->ReadDir(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // std::cout << "count = " << reply.entries().size() << std::endl;

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("ListDir: RPC Success\n");
                } 
                else 
                {
                    // std::cout << status.error_code() << ": " << status.error_message()
                    //           << std::endl;
                    //PrintErrorMessage(status.error_code(), status.error_message(), "ListDir");
                    dbgprintf("ListDir: RPC Failure\n");
                    return -1;
                }

                dbgprintf("ListDir: Exiting function\n");
                for (auto itr = reply.entries().begin(); itr != reply.entries().end(); itr++)
                {
                    struct stat st;
                    memset(&st, 0, sizeof(st));
                    st.st_ino = itr->size();
                    st.st_mode = itr->mode();
                    if (filler(buf, itr->file_name().c_str() , &st, 0, static_cast<fuse_fill_dir_flags>(0)))
                        break;
                }
                return 0;
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
                if (status.ok() && reply.status().error() == 0) 
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
                    return -reply.status().error();
                }
            }

            int DeleteFile(std::string path) 
            {
                dbgprintf("DeleteFile: Entered function\n");
                RemoveRequest request;
                RemoveResponse reply;
                Status status;
                uint32_t retryCount = 0;
                
                request.set_pathname(path);
                
                // Make RPC 
                // Retry with backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("DeleteFile: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->Remove(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE );
                

                // Checking RPC Status 
                if (status.ok()) 
                {
                    dbgprintf("DeleteFile: RPC success\n");
                    dbgprintf("DeleteFile: Exiting function\n");
                    return 0;
                } 
                else
                {
                    dbgprintf("DeleteFile: RPC failure\n");
                    dbgprintf("DeleteFile: Exiting function\n");
                    return -1;
                }                
            }

        private:
                std::unique_ptr<FileSystemService::Stub> stub_;
 
    };
}

