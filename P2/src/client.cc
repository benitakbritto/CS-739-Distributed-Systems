#include <grpcpp/grpcpp.h>
#include <string>
#include <chrono>
#include <ctime>
#include "filesystemcomm.grpc.pb.h"

#include <fcntl.h>
#include <unistd.h>


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using filesystemcomm::FileSystemService;
using filesystemcomm::OpenFileRequest;
using filesystemcomm::OpenFileResponse;
using filesystemcomm::CloseFileRequest;
using filesystemcomm::CloseFileResponse;
using filesystemcomm::DeleteFileRequest;
using filesystemcomm::DeleteFileResponse;
using filesystemcomm::GetFileStatRequest;
using filesystemcomm::GetFileStatResponse;
using filesystemcomm::MakeDirRequest;
using filesystemcomm::MakeDirResponse;
using filesystemcomm::DeleteDirRequest;
using filesystemcomm::DeleteDirResponse;
using filesystemcomm::ListDirRequest;
using filesystemcomm::ListDirResponse;



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

  // TODO: Check cache with TestAuth/GetFileStat
  // TODO: must handle open() and creat()
  FileDescriptor OpenFile(std::string path) 
  {
    int file;
    if (!TestAuth())
    {
      OpenFileRequest request;
      OpenFileResponse reply;
      ClientContext context;
      request.set_path(path);

      // Make RPC
      Status status = stub_->OpenFile(&context, request, &reply);

      // Checking RPC Status
      if (status.ok()) 
      {
        file = open(path.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0666);
        write(file, reply.data().c_str(), reply.data().length());
        close(file);
      } 
      else 
      {
        std::cout << status.error_code() << ": " << status.error_message()
                  << std::endl;
        std::cout << "OpenFile RPC Failed" << std::endl;
      }
    } //End fetch from server case
    
    // Return file descriptor
    file = open(path.c_str(), O_RDWR);
    return FileDescriptor(file, path);
  }

  // TODO: Check cache with TestAuth/GetFileStat and invoke RPC
  std::string CloseFile(FileDescriptor fd, std::string data) 
  {
  
    close(fd.file);
    
    // No RPC necessary if file wasn't modified
    if (TestAuth())
      return "Close success";
  
    CloseFileRequest request;
    CloseFileResponse reply;
    ClientContext context;
    request.set_path(fd.path);
    request.set_data(data);


    // Make RPC
    Status status = stub_->CloseFile(&context, request, &reply);

    // Checking RPC Status
    if (status.ok()) 
    {
      return reply.val();
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "CloseFile RPC Failed";
    }
  }


  // TODO -- local 
  // TODO: handle read() and pread()
  int ReadFile(FileDescriptor fd, char* buf, int length)
  {
    int read_in = read(fd.file, buf, length);
    return read_in;
  }

  // TODO -- local
  // TODO: handle write() and pwrite()
  int WriteFile(FileDescriptor fd, char* buf, int length)
  {
    int written_out = write(fd.file, buf, length);
    std::cout << buf << std::endl;
    return written_out;
  }
  
  // TODO -- local 
  // TODO: handle read() and pread()
  int ReadFile(FileDescriptor fd, char* buf, int length, int offset)
  {
    int read_in = pread(fd.file, buf, length, offset);
    return read_in;
  }

  // TODO -- local
  // TODO: handle write() and pwrite()
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
  std::string DeleteFile(std::string req) 
  {
    DeleteFileRequest request;
    DeleteFileResponse reply;
    ClientContext context;
    request.set_val(req);

    // Make RPC
    Status status = stub_->DeleteFile(&context, request, &reply);

    // Checking RPC Status 
    if (status.ok()) 
    {
      return reply.val();
    } 
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "DeleteFile RPC Failed";
    }
  }

  // TODO Get the right response type
  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * Else returns file details
  */
  std::string GetFileStat(std::string req) 
  {
    GetFileStatRequest request;
    GetFileStatResponse reply;
    ClientContext context;
    request.set_val(req);

    // Make RPC
    Status status = stub_->GetFileStat(&context, request, &reply);

    // Checking RPC Status
    if (status.ok()) 
    {
      return reply.val();
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "GetFileStat RPC Failed";
    }
  }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * else prints <TODO>
  */
  std::string MakeDir(std::string req) 
  {
    MakeDirRequest request;
    MakeDirResponse reply;
    ClientContext context;
    request.set_val(req);

    // Make RPC
    Status status = stub_->MakeDir(&context, request, &reply);

    // Checking RPC Status
    if (status.ok()) 
    {
      return reply.val();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "MakeDir RPC Failed";
    }
  }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * else prints <TODO>
  */
  std::string DeleteDir(std::string req) 
  {
    DeleteDirRequest request;
    DeleteDirResponse reply;
    ClientContext context;
    request.set_val(req);


    // Make RPC
    Status status = stub_->DeleteDir(&context, request, &reply);

    // Checking RPC Status
    if (status.ok()) 
    {
      return reply.val();
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "DeleteDir RPC Failed";
    }
  }

  // TODO: Get the right response type
  std::string ListDir(std::string req) 
  {
    ListDirRequest request;
    ListDirResponse reply;
    ClientContext context;
    request.set_val(req);


    // Make RPC
    Status status = stub_->ListDir(&context, request, &reply);

    // Checking RPC Status
    if (status.ok()) 
    {
      return reply.val();
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "ListDir RPC Failed";
    }
  }

  // TODO: Wrapper Around GetFileStat
  bool TestAuth()
  {
    return true;
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
  std::cout << "OpenFile()" << std::endl;
  FileDescriptor fd = client.OpenFile("example/hello-world.txt");
  std::cout << "Requested file open: " << request << std::endl;
  std::cout << "Response content: " << fd.file << std::endl;

  client.WriteFile(fd, "hello", 5);
  char c[10];
  int read = client.ReadFile(fd, c, 5, 0);
  for (int i = 0; i < 5; i++)
    std::cout << c[i] << std::endl;

  std::cout << "CloseFile()" << std::endl;
  request = "Testing CloseFile";
  
  auto t_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  
  response = client.CloseFile(fd,std::ctime(&t_now));
  std::cout << "Requested file close: " << request << std::endl;
  std::cout << "Response string: " << response << std::endl;
  
  std::cout << "DeleteFile()" << std::endl;
  request = "Testing DeleteFile";
  response = client.DeleteFile(request);
  std::cout << "Request string: " << request << std::endl;
  std::cout << "Response string: " << response << std::endl;

  std::cout << "GetFileStat()" << std::endl;
  request = "Testing GetFileStat";
  response = client.GetFileStat(request);
  std::cout << "Request string: " << request << std::endl;
  std::cout << "Response string: " << response << std::endl;

  std::cout << "MakeDir()" << std::endl;
  request = "Testing MakeDir";
  response = client.MakeDir(request);
  std::cout << "Request string: " << request << std::endl;
  std::cout << "Response string: " << response << std::endl;

  std::cout << "DeleteDir()" << std::endl;
  request = "Testing DeleteDir";
  response = client.DeleteDir(request);
  std::cout << "Request string: " << request << std::endl;
  std::cout << "Response string: " << response << std::endl;

  std::cout << "ListDir()" << std::endl;
  request = "Testing ListDir";
  response = client.ListDir(request);
  std::cout << "Request string: " << request << std::endl;
  std::cout << "Response string: " << response << std::endl;
}

int main(int argc, char* argv[]) 
{
  RunClient();

  return 0;
}