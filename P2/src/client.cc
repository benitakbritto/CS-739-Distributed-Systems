#include <grpcpp/grpcpp.h>
#include <string>
#include <chrono>
#include <ctime>
#include "filesystemcomm.grpc.pb.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


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
using filesystemcomm::DirectoryEntry;
using filesystemcomm::Timestamp;
using grpc::Status;
using grpc::StatusCode;

enum FileMode 
{
  UNSUPPORTED = 0,
  REG = 1,
  DIR = 2,
};

enum TestAuthMode
{
  FETCH = 0,
  STORE = 1
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

struct OpenFileReturnType
{
  Status status;
  FetchResponse response;
  FileDescriptor fd;
  OpenFileReturnType(Status status,
                    FetchResponse response,
                    FileDescriptor fd) :
                    status(status.error_code(), status.error_message()),
                    response(response),
                    fd(fd.file, fd.path)
                    {}
};

struct CloseFileReturnType
{
  Status status;
  StoreResponse response;
  CloseFileReturnType(Status status,
                      StoreResponse response) :
                      status(status),
                      response(response)
                      {}
};

class ClientImplementation 
{
 public:
  ClientImplementation(std::shared_ptr<Channel> channel)
      : stub_(FileSystemService::NewStub(channel)) {}

  OpenFileReturnType OpenFile(std::string path) 
  {
    dbgprintf("OpenFile: Inside function\n");
    int file;
    FetchRequest request;
    FetchResponse reply;
    ClientContext context;
    Status status;
    
    if (TestAuth(path, FETCH))
    {  
      request.set_pathname(path);

      // Make RPC
      dbgprintf("OpenFile: Invoking Fetch() RPC\n");
      status = stub_->Fetch(&context, request, &reply);
      dbgprintf("OpenFile: RPC Returned\n");

      // Checking RPC Status
      if (status.ok()) 
      {
        dbgprintf("OpenFile: RPC Success\n");
        file = open(path.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0666);
        dbgprintf("OpenFile: reply.file_contents().length() = %ld\n", reply.file_contents().length());
        write(file, reply.file_contents().c_str(), reply.file_contents().length());
        close(file);

        file = open(path.c_str(), O_RDWR | O_CREAT);
      } 
      else 
      {
        dbgprintf("OpenFile: RPC Failure\n");
        std::cout << status.error_code() << ": " << status.error_message()
                  << std::endl;
      }
    } 
    
    dbgprintf("OpenFile: Exiting function\n");

    return OpenFileReturnType(status,
                              reply,
                              FileDescriptor(file, path));
  }

  CloseFileReturnType CloseFile(FileDescriptor fd, std::string data) 
  {
    dbgprintf("CloseFile: Entered function\n");
    std::cout << "CloseFile: data = " << data << std::endl;
    close(fd.file);
    StoreRequest request;
    StoreResponse reply;
    ClientContext context;
    Status status;

    // No RPC necessary if file wasn't modified
    if (!TestAuth(fd.path, STORE))
    {
      dbgprintf("CloseFile: No server interaction needed\n");
      dbgprintf("CloseFile: Exiting function\n");
      return CloseFileReturnType(status,
                              reply);
    }
  
    request.set_pathname(fd.path);
    std::cout << "CloseFile: fd.path = " << fd.path << std::endl;
    request.set_file_contents(data);

    // Make RPC
    status = stub_->Store(&context, request, &reply);
    dbgprintf("CloseFile: RPC returned\n");

    // Checking RPC Status
    if (status.ok()) 
    {
       dbgprintf("CloseFile: RPC Success\n");
    } 
    else 
    {
      dbgprintf("CloseFile: RPC Failure\n");
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
    }
    dbgprintf("CloseFile: Exiting function\n");
    return CloseFileReturnType(status,
                              reply);
  }

  int ReadFile(FileDescriptor fd, char* buf, int length)
  {
    int read_in = read(fd.file, buf, length);
    return read_in;
  }

  int WriteFile(FileDescriptor fd, char* buf, int length)
  {
    int written_out = write(fd.file, buf, length);
    fsync(fd.file);
    //std::cout << buf << std::endl;
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
    fsync(fd.file);
    //std::cout << buf << std::endl;
    return written_out;
  }

  /*
  * Invokes an RPC 
  * If RPC fails, it just prints that out to stdout
  * else prints <TODO>
  */
  void DeleteFile(std::string path) 
  {
    dbgprintf("DeleteFile: Entered function\n");
    RemoveRequest request;
    RemoveResponse reply;
    ClientContext context;
    request.set_pathname(path);

    // Make RPC
    Status status = stub_->Remove(&context, request, &reply);

    // Checking RPC Status 
    if (status.ok()) 
    {
      dbgprintf("DeleteFile: Exiting function\n");
      return;
    } 
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("DeleteFile: Exiting function\n");
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

  ListDirResponse ListDir(std::string path) 
  {
    dbgprintf("ListDir: Entering function\n");
    ListDirRequest request;
    ListDirResponse reply;
    ClientContext context;
    request.set_pathname(path);

    // Make RPC
    Status status = stub_->ListDir(&context, request, &reply);
    dbgprintf("ListDir: RPC Returned\n");

    std::cout << "count = " << reply.entries().size() << std::endl;

    // Checking RPC Status
    if (status.ok()) 
    {
      dbgprintf("ListDir: Exiting function\n");
      return reply;
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("ListDir: Exiting function\n");
      return reply;
    }
  }

  /*
  * When called on Fetch,
  * TestAuth first checks if local file exits
  * if it does not exist, TestAuth returns true
  * 
  * For Fetch and Store,
  * TestAuth gets the modify time of the local file
  * and invokes the TestAuth RPC
  * if the modify times of the local and server file
  * do not match, TestAuth returns true, else false
  *
  * Error handling for GetModifyTime:
  * For eg. on Store, if the file does not exist
  * TestAuth returns false
  */
  bool TestAuth(std::string path, enum TestAuthMode mode)
  {
    // check if local file exists
    if (mode == FETCH)
    {
      if (!FileExists(path))
      {
        dbgprintf("TestAuth: Local file does not exist\n");
        return true;
      }
    }

    TestAuthRequest request;
    TestAuthResponse reply;
    ClientContext context;
    Timestamp t;
    timespec modifyTime;
    
    if (GetModifyTime(path, &modifyTime) != 0)
    {
      dbgprintf("TestAuth: Exiting function\n");
      return false;
    }

    t.set_sec(modifyTime.tv_sec);
    t.set_nsec(modifyTime.tv_nsec);

    request.set_pathname(path);
    request.mutable_time_modify()->CopyFrom(t);

    // Make RPC
    Status status = stub_->TestAuth(&context, request, &reply);
    dbgprintf("TestAuth: RPC Returned\n");
    if (status.ok()) 
    {
      dbgprintf("TestAuth: Exiting function\n");
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("TestAuth: Exiting function\n");
    }

    return reply.has_changed();
  }

 private:
  std::unique_ptr<FileSystemService::Stub> stub_;

  uint32_t GetModifyTime(std::string filepath, timespec * t) 
  {
    dbgprintf("GetModifyTime: Entering function\n");
    struct stat sb;
    if (stat(filepath.c_str(), &sb) == -1) {
      dbgprintf("GetModifyTime: Failed\n");
      return -1;
    }
    dbgprintf("GetModifyTime: Exiting function\n");
    *t = sb.st_mtim;
    return 0;
  }

  bool FileExists(std::string path)
  {
    struct stat s;   
    return (stat(path.c_str(), &s) == 0); 
  }
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
  std::cout << "Calling OpenFile()" << std::endl;
  std::string fetchPath =  "hello-world.txt";
  OpenFileReturnType openFileReturn = client.OpenFile(fetchPath);
  std::cout << "Status Code: " << openFileReturn.status.error_code()
            << " Error Message: " <<  openFileReturn.status.error_message()
            << " time_modify: " << openFileReturn.response.time_modify().sec()
            << " fd: " << openFileReturn.fd.file
            << " filepath: " << openFileReturn.fd.path
            << std::endl;

  client.WriteFile(openFileReturn.fd, "hello", 5);
  char c[10];
  int read = client.ReadFile(openFileReturn.fd, c, 5, 0);
  for (int i = 0; i < 5; i++)
    std::cout << c[i] << std::endl;
  std::cout << "Calling CloseFile()" << std::endl;
  CloseFileReturnType closeFileReturn = client.CloseFile(openFileReturn.fd, "new data");
  std::cout << "Status Code: " << closeFileReturn.status.error_code()
            << " Error Message: " <<  closeFileReturn.status.error_message()
            << " time_modify: " << closeFileReturn.response.time_modify().sec()
            << std::endl;
  // Uncomment to Test DeleteFile
  // std::cout << "Calling DeleteFile()" << std::endl;
  // std::string removePath = "try.txt";
  // client.DeleteFile(removePath);

  // Uncomment to Test Rename
  // std::cout << "Calling Rename()" << std::endl;
  // std::string oldPath = "hello-world.txt";
  // std::string newFileName = "hello-world-renamed.txt";
  // client.Rename(oldPath, newFileName);

  // Uncomment to Test GetFileStat
  // std::cout << "Caling GetFileStat()" << std::endl;
  // FileStatMetadata metadata = client.GetFileStat("hello-world.txt");
  // std::cout << metadata.file_name << "\t"
  //           << metadata.mode << "\t" 
  //           << metadata.size << "\t"
  //           << metadata.time_access_sec << "\t"
  //           << metadata.time_modify_sec << "\t"
  //           << metadata.time_change_sec << "\t" << std::endl;

  // Uncomment to Test MakeDir
  // std::cout << "Calling MakeDir()" << std::endl;
  // client.MakeDir("newDir");

  // Uncomment to Test RemoveDir
  // std::cout << "Calling RemoveDir()" << std::endl;
  // client.RemoveDir("newDir");
  
  // Uncomment to Test ListDir
  // std::cout << "Calling ListDir()" << std::endl;
  // ListDirResponse listDirResponse;
  // listDirResponse = client.ListDir("newDir");
  // for (auto itr = listDirResponse.entries().begin(); itr != listDirResponse.entries().end(); itr++)
  // {
  //   std::cout << "file_name: " << itr->file_name() << std::endl;
  //   std::cout << "mode: " << itr->mode() << std::endl;
  //   std::cout << "size: " << itr->size() << std::endl;
  //   std::cout << std::endl;
  // }

  // Uncomment to Test TestAuth
  // std::cout << "Calling TestAuth()" << std::endl;
  // enum TestAuthMode testAuthMode = FETCH;
  // bool testAuthRet = client.TestAuth("a.txt", STORE);
  // std::cout << "testAuthRet = " << testAuthRet << std::endl;
}

int main(int argc, char* argv[]) 
{
  RunClient();

  return 0;
}