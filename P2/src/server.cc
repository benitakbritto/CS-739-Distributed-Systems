#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include <string>
#include "filesystemcomm.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using filesystemcomm::StringReply;
using filesystemcomm::StringRequest;
using filesystemcomm::FileSystemService;

// Server Implementation
class ServiceImplementation final : public FileSystemService::Service 
{
  Status SendRequest(ServerContext * context, const StringRequest * request,
                     StringReply * reply) override 
  {
    std::string a = request->val();
    reply->set_val(a);
    return Status::OK;
  }
};

void RunServer() 
{
  std::string server_address("0.0.0.0:50051");
  ServiceImplementation service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which
  // communication with client takes place
  builder.RegisterService(&service);

  // Assembling the server
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on port: " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char** argv) 
{
  RunServer();
  return 0;
}