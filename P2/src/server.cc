#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include <string>
#include "filesystemcomm.grpc.pb.h"
#include <iostream>
#include <fstream>
#include <filesystem>

using std::cout;
using std::endl;
using std::string;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using filesystemcomm::FileSystemService;
using filesystemcomm::OpenFileRequest;
using filesystemcomm::OpenFileResponse;
using filesystemcomm::CloseFileRequest;
using filesystemcomm::CloseFileResponse;
using filesystemcomm::CreateFileRequest;
using filesystemcomm::CreateFileResponse;
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

string read_file(std::filesystem::path filepath) {
  std::ostringstream strm;
  std::ifstream file(filepath,std::ios::binary);
  strm << file.rdbuf();
  return strm.str();
}

void write_file(std::filesystem::path filepath, string content) {
  std::ofstream file;
  file.open(filepath,std::ios::binary);
  file << content;
  file.close();
}

// Server Implementation
class ServiceImplementation final : public FileSystemService::Service 
{
  std::filesystem::path root;
  
  public:
    ServiceImplementation(std::filesystem::path root) : root(root){}
  
  
  bool check_path(string relative, std::filesystem::path & normalized) {
    normalized = (root / relative).lexically_normal();
    auto [a,b] = std::mismatch(root.begin(),root.end(),normalized.begin());
    return a == root.end();
  }
  
  Status OpenFile(ServerContext * context, const OpenFileRequest * request,
                     OpenFileResponse * reply) override 
  {
    std::filesystem::path filepath;
    if(!check_path(request->path(), filepath)) {
      auto errc = "Failed to validate path " + request->path() + " -> " + filepath.string();
      cout << errc << endl;
      return Status(StatusCode::INVALID_ARGUMENT, errc);
    }
    
    cout << "Reading file at " << filepath << endl;
    
    // In C++, protobuf `bytes` fields are implemented as strings
    auto content = read_file(filepath);    
    reply->set_data(content);
    
    return Status::OK;
  }

  Status CloseFile(ServerContext * context, const CloseFileRequest * request,
                     CloseFileResponse * reply) override 
  {
    std::filesystem::path filepath;
    if(!check_path(request->path(), filepath)) {
      auto errc = "Failed to validate path " + request->path() + " -> " + filepath.string();
      cout << errc << endl;
      return Status(StatusCode::INVALID_ARGUMENT, errc);
    }
    
    write_file(filepath,request->data());
    reply->set_val("Wrote file!");
    
    return Status::OK;
  }

  Status CreateFile(ServerContext * context, const CreateFileRequest * request,
                     CreateFileResponse * reply) override 
  {
    reply->set_val(request->val());
    return Status::OK;
  }

  Status DeleteFile(ServerContext * context, const DeleteFileRequest * request,
                     DeleteFileResponse * reply) override 
  {
    reply->set_val(request->val());
    return Status::OK;
  }

  Status GetFileStat(ServerContext * context, const GetFileStatRequest * request,
                     GetFileStatResponse * reply) override 
  {
    reply->set_val(request->val());
    return Status::OK;
  }

  Status MakeDir(ServerContext * context, const MakeDirRequest * request,
                     MakeDirResponse * reply) override 
  {
    reply->set_val(request->val());
    return Status::OK;
  }

  Status DeleteDir(ServerContext * context, const DeleteDirRequest * request,
                     DeleteDirResponse * reply) override 
  {
    reply->set_val(request->val());
    return Status::OK;
  }

  Status ListDir(ServerContext * context, const ListDirRequest * request,
                     ListDirResponse * reply) override 
  {
    reply->set_val(request->val());
    return Status::OK;
  }
};

void RunServer(std::filesystem::path root) 
{
  ServiceImplementation service(root);
  ServerBuilder builder;

  string server_address("0.0.0.0:50051");
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on port: " << server_address << endl;

  server->Wait();
}

int main(int argc, char** argv) 
{
  if(argc != 2) {
    cout << "Usage: " << argv[0] << " root_folder" << endl;
    return 1;
  }
  
  auto root = std::filesystem::canonical(argv[1]);
  
  cout << "Serving files from " << root << endl;
  
  
  RunServer(root);
  return 0;
}