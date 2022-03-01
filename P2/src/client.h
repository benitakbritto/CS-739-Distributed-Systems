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