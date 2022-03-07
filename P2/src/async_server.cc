#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <chrono>
#include <utility>
#include "filesystemcomm.grpc.pb.h"
#include "locks.h"

/******************************************************************************
 * MACROS
 *****************************************************************************/
#define DEBUG 1
#define dbgprintf(...)       \
    if (DEBUG) {             \
        printf(__VA_ARGS__); \
    }
#define errprintf(...) \
    { printf(__VA_ARGS__); }

#define SMALL_FILE_SIZE_THRESHOLD 10000 // bytes
#define MEM_MAP_MAX_KEY_COUNT    100 // num keys in map
#define MEM_MAP_FREE_COUNT       10 // free 10 files
#define MEM_MAP_START_FREE       50 // start freeing map when keys have reached this count value
#define PERFORMANCE_MEM_MAP      0 // setting to read from mem map
#define CHUNK_SIZE               1024 // for streaming

/******************************************************************************
 * NAMESPACE
 *****************************************************************************/
namespace fs = std::filesystem;
using filesystemcomm::DirectoryEntry;
using filesystemcomm::FetchRequest;
using filesystemcomm::FetchResponse;
using filesystemcomm::FileMode;
using filesystemcomm::FileStat;
using filesystemcomm::FileSystemService;
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
using filesystemcomm::Timestamp;
using fs::path;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerCompletionQueue;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using std::cout;
using std::endl;
using std::string;
using grpc::ServerReader;
using grpc::ServerWriter;

static const string TEMP_FILE_EXT = ".afs_tmp";

class ServiceImplementation final : public FileSystemService::AsyncService {

  public:

  path to_storage_path(string relative) {
    	dbgprintf("to_storage_path: root = %s\n", root.c_str());
	    dbgprintf("to_storage_path: relative = %s\n", relative.c_str());
	    path normalized = (root / relative).lexically_normal();
        dbgprintf("to_storage_path: normalized = %s\n", normalized.c_str());
        // Check that this path is under our storage root
        auto [a, b] = std::mismatch(root.begin(), root.end(), normalized.begin());
        if (a != root.end()) {
            
        }

        if (normalized.extension() == TEMP_FILE_EXT) {
        }

        return normalized;
    }
  ServiceImplementation(path root) : root(root) {}

  ~ServiceImplementation() {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  void RunServer() {
    std::string server_address("0.0.0.0:50051");

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    HandleRpcs();
  }

  void make_dir(path filepath, int mode) {
        dbgprintf("make_dir: Entering function\n");
        if (mkdir(filepath.c_str(), mode) == -1) {
            dbgprintf("make_dir: Exiting function\n");
        }
        dbgprintf("make_dir: Exiting function\n");
    }

 private:
  path root;
  std::unique_ptr<ServerCompletionQueue> cq_;
  FileSystemService::AsyncService service_;
  std::unique_ptr<Server> server_;

  // Class encompasing the state and logic needed to serve a request.
  class CallData {
   public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(FileSystemService::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      // Invoke the serving logic right away.
      Proceed();
    }

    void Proceed() {
      if (status_ == CREATE) {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestMakeDir(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);

        // The actual processing.
        dbgprintf("MakeDir Aysnc: Entering function\n");

        dbgprintf("make_dir: Entering function\n");
        if (mkdir(request_.pathname().c_str(), S_IRWXU) == -1) {
            dbgprintf("make_dir: Exiting function\n");
        }
        dbgprintf("make_dir: Exiting function\n");
        

        dbgprintf("MakeDir Async: Exiting function on Success path\n");

        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
      } else {
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

   private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    FileSystemService::AsyncService* service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue* cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;

    MakeDirRequest request_;
    MakeDirResponse reply_;
    // The means to get back to the client.
    ServerAsyncResponseWriter<MakeDirResponse> responder_;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
  };

  // This can be run in multiple threads if needed.
  void HandleRpcs() {
    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get());
    void* tag;  // uniquely identifies a request.
    bool ok;
    while (true) {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData*>(tag)->Proceed();
    }
  }
};

int main(int argc, char** argv) {

  if (argc != 2) {
        cout << "Usage: " << argv[0] << " root_folder" << endl;
        return 1;
  }

  auto root = fs::canonical(argv[1]);

  cout << "Serving files from " << root << endl;

  ServiceImplementation service(root);
  service.RunServer();
  return 0;
}