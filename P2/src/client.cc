#include <grpcpp/grpcpp.h>
#include <string>
#include "filesystemcomm.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using filesystemcomm::StringReply;
using filesystemcomm::StringRequest;
using filesystemcomm::FileSystemService;

class ClientImplementation 
{
 public:
  ClientImplementation(std::shared_ptr<Channel> channel)
      : stub_(FileSystemService::NewStub(channel)) {}

  // Assembles client payload, sends it to the server, and returns its response
  std::string SendRequest(std::string a) 
  {
    // Data to be sent to server
    StringRequest request;
    request.set_val(a);

    // Container for server response
    StringReply reply;

    // Context can be used to send meta data to server or modify RPC behaviour
    ClientContext context;

    // Actual Remote Procedure Call
    Status status = stub_->SendRequest(&context, request, &reply);

    // Returns results based on RPC status
    if (status.ok()) 
    {
      return reply.val();
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC Failed";
    }
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
  std::string a = "grpc is cool!";

  // RPC is created and response is stored
  response = client.SendRequest(a);

  // Prints results
  std::cout << "Original string: " << a << std::endl;
  std::cout << "Reversed string: " << response << std::endl;
}

int main(int argc, char* argv[]) 
{
  RunClient();

  return 0;
}