#include <grpcpp/grpcpp.h>
#include <string>
#include <chrono>
#include <ctime>
#include "filesystemcomm.grpc.pb.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>

#define DEBUG               1
#define dbgprintf(...)      if (DEBUG) { printf(__VA_ARGS__); }
#define SERVER_ADDR         "20.69.154.6:50051"

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

struct DeletFileReturnType
{
  Status status;
  DeletFileReturnType(Status status) : status(status) {}
};

struct RenameReturnType
{
  Status status;
  RenameReturnType(Status status) : status(status) {}
};

struct FileStatReturnType
{
  Status status;
  GetFileStatResponse response;
  FileStatReturnType(Status status,
                    GetFileStatResponse response) :
                    status(status),
                    response(response)
                    {}
};

struct TestAuthReturnType
{
  Status status;
  TestAuthResponse response;
  TestAuthReturnType(Status status,
                    TestAuthResponse response) :
                    status(status),
                    response(response)
                    {}
};  

struct MakeDirReturnType
{
  Status status;
  MakeDirReturnType(Status status) : status(status) {}
};

struct RemoveDirReturnType
{
  Status status;
  RemoveDirReturnType(Status status) : status(status) {}
};

struct ListDirReturnType
{
  Status status;
  ListDirResponse response;
  ListDirReturnType(Status status,
                    ListDirResponse response) :
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
    struct utimbuf ubuf;
    
    if (TestAuth(path, FETCH).response.has_changed())
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

        //  Update local file modify time with servers modify time
        ubuf.modtime = reply.time_modify().sec();
        if (utime(path.c_str(), &ubuf) != 0)
        {
          dbgprintf("OpenFile: utime() failed\n");
        }
        
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
    struct utimbuf ubuf;

    // No RPC necessary if file wasn't modified
    if (!TestAuth(fd.path, STORE).response.has_changed())
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

      // Update local file modify time with servers modify time 
      ubuf.modtime = reply.time_modify().sec();
      if (utime(fd.path.c_str(), &ubuf) != 0)
      {
        dbgprintf("CloseFile: utime() failed\n");
      }
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

  DeletFileReturnType DeleteFile(std::string path) 
  {
    dbgprintf("DeleteFile: Entered function\n");
    RemoveRequest request;
    RemoveResponse reply;
    ClientContext context;
    Status status;
    
    request.set_pathname(path);
    
    // Make RPC
    status = stub_->Remove(&context, request, &reply);

    // Checking RPC Status 
    if (status.ok()) 
    {
      dbgprintf("DeleteFile: RPC success\n");
    } 
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("DeleteFile: RPC failure\n");
    }

    dbgprintf("DeleteFile: Exiting function\n");
    return DeletFileReturnType(status);
  }

  RenameReturnType Rename(std::string path, std::string newFileName) 
  {
    dbgprintf("Rename: Entered function\n");
    RenameRequest request;
    RenameResponse reply;
    ClientContext context;
    Status status;

    request.set_pathname(path);
    request.set_componentname(newFileName);

    // Make RPC
    status = stub_->Rename(&context, request, &reply);

    // Checking RPC Status 
    if (status.ok()) 
    {
      dbgprintf("Rename: RPC Success\n");
    } 
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("Rename: RPC failure\n");
    }
     dbgprintf("Rename: Exiting function\n");
     return RenameReturnType(status);
  }

  FileStatReturnType GetFileStat(std::string path) 
  {
    dbgprintf("GetFileStat: Entering function\n");
    GetFileStatRequest request;
    GetFileStatResponse reply;
    ClientContext context;
    Status status;

    request.set_pathname(path);

    // Make RPC
    status = stub_->GetFileStat(&context, request, &reply);
    dbgprintf("GetFileStat: RPC returned\n");

    // Checking RPC Status
    if (status.ok()) 
    {
      dbgprintf("GetFileStat: RPC Success\n");
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("GetFileStat: RPC Failed\n");
    }
    
    dbgprintf("GetFileStat: Exiting function\n");
    return FileStatReturnType(status, reply);
  }

  MakeDirReturnType MakeDir(std::string path) 
  {
    dbgprintf("MakeDir: Entering function\n");
    MakeDirRequest request;
    MakeDirResponse reply;
    ClientContext context;
    Status status;
    
    request.set_pathname(path);

    // Make RPC
    status = stub_->MakeDir(&context, request, &reply);
    dbgprintf("MakeDir: RPC returned\n");

    // Checking RPC Status
    if (status.ok()) 
    {
      dbgprintf("MakeDir: RPC Success\n");
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("MakeDir: RPC failure\n");
    }

    dbgprintf("MakeDir: Exiting function\n");
    return MakeDirReturnType(status);
  }

  RemoveDirReturnType RemoveDir(std::string path) 
  {
    dbgprintf("RemoveDir: Entering function\n");
    RemoveDirRequest request;
    RemoveDirResponse reply;
    ClientContext context;
    Status status;

    request.set_pathname(path);

    // Make RPC
    status = stub_->RemoveDir(&context, request, &reply);
    dbgprintf("RemoveDir: RPC Returned\n");

    // Checking RPC Status
    if (status.ok()) 
    {
      dbgprintf("RemoveDir: RPC Success\n");
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("RemoveDir: RPC Failure\n");
    }
    dbgprintf("RemoveDir: Exiting function\n");
    return RemoveDirReturnType(status);
  }

  ListDirReturnType ListDir(std::string path) 
  {
    dbgprintf("ListDir: Entering function\n");
    ListDirRequest request;
    ListDirResponse reply;
    ClientContext context;
    Status status;

    request.set_pathname(path);

    // Make RPC
    status = stub_->ListDir(&context, request, &reply);
    dbgprintf("ListDir: RPC Returned\n");

    // std::cout << "count = " << reply.entries().size() << std::endl;

    // Checking RPC Status
    if (status.ok()) 
    {
      dbgprintf("ListDir: RPC Success\n");
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("ListDir: RPC Failure\n");
    }

    dbgprintf("ListDir: Exiting function\n");
    return ListDirReturnType(status, reply);
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
  TestAuthReturnType TestAuth(std::string path, enum TestAuthMode mode)
  {
    dbgprintf("TestAuth: Entering function\n");
    TestAuthRequest request;
    TestAuthResponse reply;
    ClientContext context;
    Timestamp t;
    timespec modifyTime;
    Status status;

    // check if local file exists
    if (mode == FETCH)
    {
      if (!FileExists(path))
      {
        dbgprintf("TestAuth: Local file does not exist\n");
        reply.set_has_changed(true);
        return TestAuthReturnType(status, reply);
      }
    }

    if (GetModifyTime(path, &modifyTime) != 0)
    {
      dbgprintf("TestAuth: Exiting function\n");
      reply.set_has_changed(false);
      return TestAuthReturnType(status, reply);
    }

    t.set_sec(modifyTime.tv_sec);
    t.set_nsec(modifyTime.tv_nsec);

    request.set_pathname(path);
    request.mutable_time_modify()->CopyFrom(t);

    // Make RPC
    status = stub_->TestAuth(&context, request, &reply);
    dbgprintf("TestAuth: RPC Returned\n");

    if (status.ok()) 
    {
      dbgprintf("TestAuth: RPC Success\n");
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      dbgprintf("TestAuth: RPC Failure\n");
    }

    dbgprintf("TestAuth: Exiting function\n");
    return TestAuthReturnType(status, reply);
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
  std::string target_address(SERVER_ADDR);
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
  // std::cout << "Calling OpenFile()" << std::endl;
  // std::string fetchPath =  "hello-world.txt";
  // OpenFileReturnType openFileReturn = client.OpenFile(fetchPath);
  // std::cout << "Status Code: " << openFileReturn.status.error_code()
  //           << " Error Message: " <<  openFileReturn.status.error_message()
  //           << " time_modify: " << openFileReturn.response.time_modify().sec()
  //           << " fd: " << openFileReturn.fd.file
  //           << " filepath: " << openFileReturn.fd.path
  //           << std::endl;

  // client.WriteFile(openFileReturn.fd, "hello", 5);
  // char c[10];
  // int read = client.ReadFile(openFileReturn.fd, c, 5, 0);
  // for (int i = 0; i < 5; i++)
  //   std::cout << c[i] << std::endl;
  // std::cout << "Calling CloseFile()" << std::endl;
  // CloseFileReturnType closeFileReturn = client.CloseFile(openFileReturn.fd, "new data");
  // std::cout << "Status Code: " << closeFileReturn.status.error_code()
  //           << " Error Message: " <<  closeFileReturn.status.error_message()
  //           << " time_modify: " << closeFileReturn.response.time_modify().sec()
  //           << std::endl;

  // Uncomment to Test DeleteFile
  // std::cout << "Calling DeleteFile()" << std::endl;
  // std::string removePath = "try.txt";
  // DeletFileReturnType deleteFileReturn = client.DeleteFile(removePath);
  // std::cout << "Status Code: " << deleteFileReturn.status.error_code()
  //           << " Error Message: " <<  deleteFileReturn.status.error_message()
  //           << std::endl;

  // Uncomment to Test Rename
  // std::cout << "Calling Rename()" << std::endl;
  // std::string oldPath = "hello-world.txt";
  // std::string newFileName = "hello-world-renamed.txt";
  // RenameReturnType renameReturn = client.Rename(oldPath, newFileName);
  // std::cout << "Status Code: " << renameReturn.status.error_code()
  //           << " Error Message: " <<  renameReturn.status.error_message()
  //           << std::endl;

  // Uncomment to Test GetFileStat
  // std::cout << "Calling GetFileStat()" << std::endl;
  // FileStatReturnType fileStatReturn = client.GetFileStat("hello-world.txt");
  // std::cout << "Status Code: " << fileStatReturn.status.error_code()
  //           << " Error Message: " <<  fileStatReturn.status.error_message()
  //           << fileStatReturn.response.status().file_name() << "\t"
  //           << fileStatReturn.response.status().mode() << "\t"
  //           << fileStatReturn.response.status().size() << "\t"
  //           << fileStatReturn.response.status().time_access().sec() << "\t"
  //           << fileStatReturn.response.status().time_access().nsec() << "\t"
  //           << fileStatReturn.response.status().time_modify().sec() << "\t"
  //           << fileStatReturn.response.status().time_modify().nsec() << "\t"
  //           << fileStatReturn.response.status().time_change().sec() << "\t"
  //           << fileStatReturn.response.status().time_change().nsec()
  //           << std::endl;

  // Uncomment to Test MakeDir
  // std::cout << "Calling MakeDir()" << std::endl;
  // MakeDirReturnType makeDirReturn = client.MakeDir("newDir");
  // std::cout << "Status Code: " << makeDirReturn.status.error_code()
  //           << " Error Message: " <<  makeDirReturn.status.error_message()
  //           << std::endl;

  // Uncomment to Test RemoveDir
  // std::cout << "Calling RemoveDir()" << std::endl;
  // RemoveDirReturnType removeDirReturn = client.RemoveDir("newDir1");
  // std::cout << "Status Code: " << removeDirReturn.status.error_code()
  //           << " Error Message: " <<  removeDirReturn.status.error_message()
  //           << std::endl;
  
  // Uncomment to Test ListDir
  // std::cout << "Calling ListDir()" << std::endl;
  // ListDirReturnType listDirReturn = client.ListDir("newDir");
  // std::cout << "Status Code: " << listDirReturn.status.error_code()
  //           << " Error Message: " <<  listDirReturn.status.error_message()
  //           << std::endl;
  // for (auto itr = listDirReturn.response.entries().begin(); itr != listDirReturn.response.entries().end(); itr++)
  // {
  //   std::cout << "file_name: " << itr->file_name() << std::endl;
  //   std::cout << "mode: " << itr->mode() << std::endl;
  //   std::cout << "size: " << itr->size() << std::endl;
  //   std::cout << std::endl;
  // }

  // Uncomment to Test TestAuth
  // std::cout << "Calling TestAuth()" << std::endl;
  // enum TestAuthMode testAuthMode = FETCH;
  // TestAuthReturnType testAuthRet = client.TestAuth("hello-world.txt", testAuthMode);
  // std::cout << "Status Code: " << testAuthRet.status.error_code()
  //           << " Error Message: " <<  testAuthRet.status.error_message()
  //           << " has_changed: " << testAuthRet.response.has_changed()
  //           << std::endl;
}

int main(int argc, char* argv[]) 
{
  RunClient();

  return 0;
}