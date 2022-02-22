#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include <string>
#include "filesystemcomm.grpc.pb.h"
#include <iostream>
#include <fstream>

using std::cout;
using std::endl;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

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

std::string read_file(std::string filename) {
  std::ostringstream strm;
  std::ifstream file(filename,std::ios::binary);
  strm << file.rdbuf();
  return strm.str();
}

// Server Implementation
class ServiceImplementation final : public FileSystemService::Service 
{
  Status OpenFile(ServerContext * context, const OpenFileRequest * request,
                     OpenFileResponse * reply) override 
  {
    auto filename = request->path();
    
    // In C++, protobuf `bytes` fields are implemented as strings
    auto content = read_file(filename);    
    reply->set_data(content);
    
    return Status::OK;
  }

  Status CloseFile(ServerContext * context, const CloseFileRequest * request,
                     CloseFileResponse * reply) override 
  {
    reply->set_val(request->val());
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

void RunServer() 
{
  ServiceImplementation service;
  ServerBuilder builder;

  std::string server_address("0.0.0.0:50051");
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on port: " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char** argv) 
{
  RunServer();
  return 0;
}