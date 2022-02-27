#include <grpcpp/grpcpp.h>
#include <string>
#include <chrono>
#include <ctime>
#include "filesystemcomm.grpc.pb.h"
#include <fcntl.h>
#include <unistd.h>

/****************************************************************************** 
* Macros
*****************************************************************************/
#define DEBUG               1
#define dbgprintf(...)      if (DEBUG) { printf(__VA_ARGS__); }

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

// TODO: Add more attributes as required
struct FileUpdateMetadata
{
  time_t accessTime; 
  time_t modifyTime;
};

struct FileDescriptor
{
  int file;
  std::string path;
  FileDescriptor(int file, std::string path) : path(path), file(file) {}
};

class ClientImplementation 
{
 public:
  ClientImplementation(std::shared_ptr<Channel> channel)
      : stub_(FileSystemService::NewStub(channel)) {}

  // Check cache with TestAuth/GetFileStat
  // must handle open() and creat()
  FileDescriptor Fetch(std::string path) 
  {
    dbgprintf("Fetch: Inside function\n");
    int file;
    // TODO: Change TestAuth signature
    // TODO: Check if local file exists
    if (!TestAuth())
    {
      FetchRequest request;
      FetchResponse reply;
      ClientContext context;
      request.set_pathname(path);

      // Make RPC
      dbgprintf("Fetch: Invoking Fetch() RPC\n");
      Status status = stub_->Fetch(&context, request, &reply);
      dbgprintf("Fetch: RPC Returned\n");

      // Checking RPC Status
      // TODO: Do something w reply.time_modify()
      if (status.ok()) 
      {
        dbgprintf("Fetch: RPC Success\n");
        file = open(path.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0666);
        dbgprintf("Fetch: reply.file_contents().length() = %ld\n", reply.file_contents().length());
        write(file, reply.file_contents().c_str(), reply.file_contents().length());
        close(file);
      } 
      else 
      {
        dbgprintf("Fetch: RPC Failure\n");
        std::cout << status.error_code() << ": " << status.error_message()
                  << std::endl;
      }
    } //End fetch from server case
    
    // Return file descriptor
    file = open(path.c_str(), O_RDWR);
    dbgprintf("Fetch: Exiting function\n");
    return FileDescriptor(file, path);
  }

  void Store(FileDescriptor fd, std::string data) 
  {
    dbgprintf("Store: Entered function\n");
    std::cout << "Store: data = " << data << std::endl;
    close(fd.file);
    
    // TODO: Update TestAuth
    // No RPC necessary if file wasn't modified
    if (TestAuth())
    {
      dbgprintf("Store: No server interaction needed\n");
      dbgprintf("Store: Exiting function\n");
      return;
    }
  
    StoreRequest request;
    StoreResponse reply;
    ClientContext context;
    request.set_pathname(fd.path);
    std::cout << "Store: fd.path = " << fd.path << std::endl;
    request.set_file_contents(data);

    // Make RPC
    Status status = stub_->Store(&context, request, &reply);
    dbgprintf("Store: RPC returned\n");

    // TODO: What to do with response
    // Checking RPC Status
    if (status.ok()) 
    {
      dbgprintf("Store: Exiting function\n");
      return;
    } 
    else 
    {
      dbgprintf("Store: RPC Failure\n");
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("Store: Exiting function\n");
      return;
    }
  }

  int ReadFile(FileDescriptor fd, char* buf, int length)
  {
    int read_in = read(fd.file, buf, length);
    return read_in;
  }

  int WriteFile(FileDescriptor fd, char* buf, int length)
  {
    int written_out = write(fd.file, buf, length);
    std::cout << buf << std::endl;
    return written_out;
  }
  
  int ReadFile(FileDescriptor fd, char* buf, int length, int offset)
  {
    int read_in = pread(fd.file, buf, length, offset);
    return read_in;
  }

  int WriteFile(FileDescriptor fd, char* buf, int length, int offset)
  {
    int written_out = pwrite(fd.file, buf, length, offset);
    std::cout << buf << std::endl;
    return written_out;
  }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * else prints <TODO>
  */
  // std::string DeleteFile(std::string req) 
  // {
  //   DeleteFileRequest request;
  //   DeleteFileResponse reply;
  //   ClientContext context;
  //   request.set_val(req);

  //   // Make RPC
  //   Status status = stub_->DeleteFile(&context, request, &reply);

  //   // Checking RPC Status 
  //   if (status.ok()) 
  //   {
  //     return reply.val();
  //   } 
  //   else
  //   {
  //     std::cout << status.error_code() << ": " << status.error_message()
  //               << std::endl;
  //     return "DeleteFile RPC Failed";
  //   }
  // }

  // TODO Get the right response type
  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * Else returns file details
  */
  // std::string GetFileStat(std::string req) 
  // {
  //   GetFileStatRequest request;
  //   GetFileStatResponse reply;
  //   ClientContext context;
  //   request.set_val(req);

  //   // Make RPC
  //   Status status = stub_->GetFileStat(&context, request, &reply);

  //   // Checking RPC Status
  //   if (status.ok()) 
  //   {
  //     return reply.val();
  //   } 
  //   else 
  //   {
  //     std::cout << status.error_code() << ": " << status.error_message()
  //               << std::endl;
  //     return "GetFileStat RPC Failed";
  //   }
  // }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * else prints <TODO>
  */
  // std::string MakeDir(std::string req) 
  // {
  //   MakeDirRequest request;
  //   MakeDirResponse reply;
  //   ClientContext context;
  //   request.set_val(req);

  //   // Make RPC
  //   Status status = stub_->MakeDir(&context, request, &reply);

  //   // Checking RPC Status
  //   if (status.ok()) 
  //   {
  //     return reply.val();
  //   }
  //   else
  //   {
  //     std::cout << status.error_code() << ": " << status.error_message()
  //               << std::endl;
  //     return "MakeDir RPC Failed";
  //   }
  // }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * else prints <TODO>
  */
  // std::string DeleteDir(std::string req) 
  // {
  //   DeleteDirRequest request;
  //   DeleteDirResponse reply;
  //   ClientContext context;
  //   request.set_val(req);


  //   // Make RPC
  //   Status status = stub_->DeleteDir(&context, request, &reply);

  //   // Checking RPC Status
  //   if (status.ok()) 
  //   {
  //     return reply.val();
  //   } 
  //   else 
  //   {
  //     std::cout << status.error_code() << ": " << status.error_message()
  //               << std::endl;
  //     return "DeleteDir RPC Failed";
  //   }
  // }

  // TODO: Get the right response type
  // std::string ListDir(std::string req) 
  // {
  //   ListDirRequest request;
  //   ListDirResponse reply;
  //   ClientContext context;
  //   request.set_val(req);


  //   // Make RPC
  //   Status status = stub_->ListDir(&context, request, &reply);

  //   // Checking RPC Status
  //   if (status.ok()) 
  //   {
  //     return reply.val();
  //   } 
  //   else 
  //   {
  //     std::cout << status.error_code() << ": " << status.error_message()
  //               << std::endl;
  //     return "ListDir RPC Failed";
  //   }
  // }

  // TODO
  bool TestAuth()
  {
    return false;
  }

 private:
  std::unique_ptr<FileSystemService::Stub> stub_;
};

void RunClient() 
{
  std::string target_address("0.0.0.0:50051");
  // Instantiates the client
  ClientImplementation client(
      // Channel from which RPCs are made - endpoint is the target_address
      grpc::CreateChannel(target_address,
                          // Indicate when channel is not authenticated
                          grpc::InsecureChannelCredentials()));

  std::string response;
  std::string request;

  // Client RPC invokation
  std::cout << "Calling Fetch()" << std::endl;
  std::string fetchPath =  "hello-world.txt";
  FileDescriptor fd = client.Fetch(fetchPath);
  std::cout << "Request (file path): " << fetchPath << std::endl;
  std::cout << "Response (file descriptor): " << fd.file << std::endl;

  client.WriteFile(fd, "hello", 5);
  char c[10];
  int read = client.ReadFile(fd, c, 5, 0);
  for (int i = 0; i < 5; i++)
    std::cout << c[i] << std::endl;

  std::cout << "Calling Store()" << std::endl;
  client.Store(fd, "new data");
  
  // std::cout << "DeleteFile()" << std::endl;
  // request = "Testing DeleteFile";
  // response = client.DeleteFile(request);
  // std::cout << "Request string: " << request << std::endl;
  // std::cout << "Response string: " << response << std::endl;

  // std::cout << "GetFileStat()" << std::endl;
  // request = "Testing GetFileStat";
  // response = client.GetFileStat(request);
  // std::cout << "Request string: " << request << std::endl;
  // std::cout << "Response string: " << response << std::endl;

  // std::cout << "MakeDir()" << std::endl;
  // request = "Testing MakeDir";
  // response = client.MakeDir(request);
  // std::cout << "Request string: " << request << std::endl;
  // std::cout << "Response string: " << response << std::endl;

  // std::cout << "DeleteDir()" << std::endl;
  // request = "Testing DeleteDir";
  // response = client.DeleteDir(request);
  // std::cout << "Request string: " << request << std::endl;
  // std::cout << "Response string: " << response << std::endl;

  // std::cout << "ListDir()" << std::endl;
  // request = "Testing ListDir";
  // response = client.ListDir(request);
  // std::cout << "Request string: " << request << std::endl;
  // std::cout << "Response string: " << response << std::endl;
}

int main(int argc, char* argv[]) 
{
  RunClient();

  return 0;
}