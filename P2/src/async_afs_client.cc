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
#include <fstream>
#include <sstream>

/******************************************************************************
 * MACROS
 *****************************************************************************/
#define DEBUG                 1                                     // for debugging
#define dbgprintf(...)        if (DEBUG) { printf(__VA_ARGS__); }   // for debugging
#define MAX_RETRY             5                                     // rpc retry
#define RETRY_TIME_START      1                                     // in seconds
#define RETRY_TIME_MULTIPLIER 2                                     // for rpc retry w backoff
#define LOCAL_CACHE_PREFIX    "/tmp/afs/"                           // location of local files
#define CHUNK_SIZE            1024                                  // for streaming
//#define SERVER_ADDR         "52.151.53.152:50051"                 // Server: VM1
//#define SERVER_ADDR         "20.69.154.6:50051"                   // Server: VM2
//#define SERVER_ADDR         "20.69.94.59:50051"                   // Server: VM3
#define SERVER_ADDR           "0.0.0.0:50051"                       // Server: self
#define PERFORMANCE           0                                     // set to 1 to run performant functions
#define CRASH_TEST                                                  //Remove to disable all crashes
#define SINGLE_LOG            1                                     // Turns on single log functionality

#ifdef CRASH_TEST
#define crash(...) if (__VA_ARGS__) *((char*)0) = 0; else do {} while(0)
#else
#define crash() do {} while(0)
#endif

/******************************************************************************
 * NAMESPACES
 *****************************************************************************/
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientAsyncResponseReader;
using grpc::CompletionQueue;
using filesystemcomm::FileSystemService;
using filesystemcomm::FetchRequest;
using filesystemcomm::FetchResponse;
using filesystemcomm::GetFileStatRequest;
using filesystemcomm::GetFileStatResponse;
using filesystemcomm::ListDirRequest;
using filesystemcomm::ListDirResponse;
using filesystemcomm::MakeDirRequest;
using filesystemcomm::MakeDirResponse;
using filesystemcomm::MknodRequest;
using filesystemcomm::MknodResponse;
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
using std::ifstream;
using std::ostringstream;
using grpc::ClientWriter;
using grpc::ClientReader;

namespace FileSystemClient
{
    class ClientAsyncImplementation 
    {
        public:
            ClientAsyncImplementation(std::shared_ptr<Channel> channel)
                : stub_(FileSystemService::NewStub(channel)) {}
            
            // Convert an RPC failure status code to something like an errno
            int transform_rpc_err(grpc::StatusCode code) {
                switch(code) {
                    case StatusCode::OK:
                        return 0;
                    case StatusCode::INVALID_ARGUMENT:
                        return EINVAL;
                    case StatusCode::NOT_FOUND:
                        return ENOENT;
                    case StatusCode::DEADLINE_EXCEEDED:
                    case StatusCode::UNAVAILABLE:
                        return ETIMEDOUT;
                    case StatusCode::ALREADY_EXISTS:
                        return EEXIST;
                    case StatusCode::PERMISSION_DENIED:
                    case StatusCode::UNAUTHENTICATED:
                        return EPERM;
                    default:
                        return EIO; // fall back to IO err for unexpected issues
                }
            }
            
            int MakeDir(std::string path, mode_t mode) 
            {
                dbgprintf("MakeDir Async: Entering function\n");
                MakeDirRequest request;
                MakeDirResponse reply;
                Status status;
                // Data we are sending to the server.
                request.set_pathname(path);
                ClientContext context;
                CompletionQueue cq;
                
                std::unique_ptr<ClientAsyncResponseReader<MakeDirResponse> > rpc(
                    stub_->PrepareAsyncMakeDir(&context, request, &cq));
                    
                rpc->StartCall();
                rpc->Finish(&reply, &status, (void*)1);
                
                void* got_tag;
                bool ok = false;
                
                GPR_ASSERT(cq.Next(&got_tag, &ok));
                
                GPR_ASSERT(got_tag == (void*)1);
                GPR_ASSERT(ok);

                if (status.ok()) {
                    return 0;
                } else {
                    return -1;
                }
            }
           
                
        private:
            unique_ptr<FileSystemService::Stub> stub_;
    };
}

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  FileSystemClient::ClientAsyncImplementation cli(grpc::CreateChannel(
      "0.0.0.0:50051", grpc::InsecureChannelCredentials()));
  std::string path("abc");
  int reply = cli.MakeDir(path, S_IRWXU);  // The actual RPC call!
  std::cout << "Client received: " << reply << std::endl;

  return 0;
}

