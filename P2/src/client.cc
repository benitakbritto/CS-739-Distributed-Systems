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

enum FileMode 
{
  UNSUPPORTED = 0,
  REG = 1,
  DIR = 2,
};

struct FileStatMetadata
{
  std::string file_name;
  uint32_t mode;
  uint64_t size;
  uint64_t time_access_sec; // seconds
  uint64_t time_modify_sec; // seconds
  uint64_t time_change_sec; // seconds
  FileStatMetadata(std::string file_name, 
                    uint32_t mode, 
                    uint64_t size, 
                    uint64_t time_access_sec, 
                    uint64_t time_modify_sec,
                    uint64_t time_change_sec) :
                    file_name(file_name),
                    mode(mode),
                    size(size),
                    time_access_sec(time_access_sec),
                    time_modify_sec(time_modify_sec),
                    time_change_sec(time_change_sec)
                    {}
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
  void Remove(std::string path) 
  {
    dbgprintf("Remove: Entered function\n");
    RemoveRequest request;
    RemoveResponse reply;
    ClientContext context;
    request.set_pathname(path);

    // Make RPC
    Status status = stub_->Remove(&context, request, &reply);

    // Checking RPC Status 
    if (status.ok()) 
    {
      dbgprintf("Remove: Exiting function\n");
      return;
    } 
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("Remove: Exiting function\n");
    }
  }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * else prints <TODO>
  */
  void Rename(std::string path, std::string newFileName) 
  {
    dbgprintf("Rename: Entered function\n");
    RenameRequest request;
    RenameResponse reply;
    ClientContext context;
    request.set_pathname(path);
    request.set_componentname(newFileName);

    // Make RPC
    Status status = stub_->Rename(&context, request, &reply);

    // Checking RPC Status 
    if (status.ok()) 
    {
      dbgprintf("Rename: Exiting function\n");
      return;
    } 
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("Rename: Exiting function\n");
      return;
    }
  }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * Else returns file details
  */
  FileStatMetadata GetFileStat(std::string path) 
  {
    dbgprintf("GetFileStat: Entering function\n");
    GetFileStatRequest request;
    GetFileStatResponse reply;
    ClientContext context;
    request.set_pathname(path);

    // Make RPC
    Status status = stub_->GetFileStat(&context, request, &reply);
    dbgprintf("GetFileStat: RPC returned\n");

    // Checking RPC Status
    if (status.ok()) 
    {
      dbgprintf("GetFileStat: Exiting function\n");
      return FileStatMetadata(reply.status().file_name(),
                              reply.status().mode(),
                              reply.status().size(),
                              reply.status().time_access().sec(),
                              reply.status().time_modify().sec(),
                              reply.status().time_change().sec());
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("GetFileStat: Exiting function\n");
      return FileStatMetadata("", UNSUPPORTED, 0, 0, 0, 0);
    }
  }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * else prints <TODO>
  */
  void MakeDir(std::string path) 
  {
    dbgprintf("MakeDir: Entering function\n");
    MakeDirRequest request;
    MakeDirResponse reply;
    ClientContext context;
    request.set_pathname(path);

    // Make RPC
    Status status = stub_->MakeDir(&context, request, &reply);
    dbgprintf("MakeDir: RPC returned\n");

    // Checking RPC Status
    if (status.ok()) 
    {
      dbgprintf("MakeDir: Exiting function\n");
      return;
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("MakeDir: Exiting function\n");
      return;
    }
  }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * else prints <TODO>
  */
  void RemoveDir(std::string path) 
  {
    dbgprintf("RemoveDir: Entering function\n");
    RemoveDirRequest request;
    RemoveDirResponse reply;
    ClientContext context;
    request.set_pathname(path);

    // Make RPC
    Status status = stub_->RemoveDir(&context, request, &reply);
    dbgprintf("RemoveDir: RPC Returned\n");

    // Checking RPC Status
    if (status.ok()) 
    {
      dbgprintf("RemoveDir: Exiting function\n");
      return;
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("RemoveDir: Exiting function\n");
      return;
    }
  }

  // TODO
  // std::string ListDir(std::string path) 
  // {
  //   dbgprintf("ListDir: Entering function\n");
  //   ListDirRequest request;
  //   ListDirResponse reply;
  //   ClientContext context;
  //   request.set_pathname(path);


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
  // Uncomment to Test Open-Read-Write-Close
  // std::cout << "Calling Fetch()" << std::endl;
  // std::string fetchPath =  "hello-world.txt";
  // FileDescriptor fd = client.Fetch(fetchPath);
  // std::cout << "Request (file path): " << fetchPath << std::endl;
  // std::cout << "Response (file descriptor): " << fd.file << std::endl;
  // client.WriteFile(fd, "hello", 5);
  // char c[10];
  // int read = client.ReadFile(fd, c, 5, 0);
  // for (int i = 0; i < 5; i++)
  //   std::cout << c[i] << std::endl;
  // std::cout << "Calling Store()" << std::endl;
  // client.Store(fd, "new data");
  
  // Uncomment to Test Remove
  // std::cout << "Calling Remove()" << std::endl;
  // std::string removePath = "try.txt";
  // client.Remove(removePath);

  // Uncomment to Test Rename
  // std::cout << "Calling Rename()" << std::endl;
  // std::string oldPath = "hello-world.txt";
  // std::string newFileName = "hello-world-renamed.txt";
  // client.Rename(oldPath, newFileName);

  std::cout << "Caling GetFileStat()" << std::endl;
  FileStatMetadata metadata = client.GetFileStat("hello-world.txt");
  std::cout << metadata.file_name << "\t"
            << metadata.mode << "\t" 
            << metadata.size << "\t"
            << metadata.time_access_sec << "\t"
            << metadata.time_modify_sec << "\t"
            << metadata.time_change_sec << "\t" << std::endl;

  // Uncomment to Test MakeDir
  // std::cout << "Calling MakeDir()" << std::endl;
  // client.MakeDir("newDir");

  // Uncomment to Test RemoveDir
  // std::cout << "Calling RemoveDir()" << std::endl;
  // client.RemoveDir("newDir");

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