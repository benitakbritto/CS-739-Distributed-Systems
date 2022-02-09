#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReader;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;
using helloworld::IntRequest;
using helloworld::DoubleRequest;
using helloworld::StringRequest;
using helloworld::CustomResponse;
using helloworld::ComplexDataStructureRequest;

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service 
{
  Status AcceptInt(ServerContext* context, const IntRequest* request,
                       CustomResponse* reply) override 
  {
    //reply->set_val("Received " + std::to_string(request->val()));
    reply->set_val("Received");
    return Status::OK;
  }

  Status AcceptDouble(ServerContext* context, const DoubleRequest* request,
                       CustomResponse* reply) override 
  {
    // reply->set_val("Received " + std::to_string(request->val()));
    reply->set_val("Received");
    return Status::OK;
  }

  Status AcceptString(ServerContext* context, const StringRequest* request,
                       CustomResponse* reply) override 
  {
    // reply->set_val("Received " + request->val());
    reply->set_val("Received");
    return Status::OK;
  }

  Status AcceptComplexDataStructure(ServerContext* context, const ComplexDataStructureRequest* request,
                       CustomResponse* reply) override {
    // reply->set_val("Received " + std::to_string(request->val1()) + "," + std::to_string(request->val2()) + "," + request->val3());
    reply->set_val("Received");
    return Status::OK;
  }

  Status AcceptClientSideStream(ServerContext* context, ServerReader<StringRequest>* reader,
                     CustomResponse* reply) override 
  {
    StringRequest currentStr;
    while (reader->Read(&currentStr)) 
    {
      // do nothing
    }

    reply->set_val("Received");
    return Status::OK;
  }

};

void RunServer() 
{
  std::string server_address("0.0.0.0:50051");
  GreeterServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) 
{
  RunServer();

  return 0;
}
