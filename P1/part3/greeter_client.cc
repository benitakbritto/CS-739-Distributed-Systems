/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <time.h>

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientWriter;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;
using helloworld::IntRequest;
using helloworld::DoubleRequest;
using helloworld::StringRequest;
using helloworld::CustomResponse;
using helloworld::ComplexDataStructureRequest;

class GreeterClient 
{
 public:  
  GreeterClient(std::shared_ptr<Channel> channel) : stub_(Greeter::NewStub(channel)) {}
  
  std::string AcceptInt(const std::int32_t& req) 
  {
    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    IntRequest request;
    request.set_val(req);
    clock_gettime(CLOCK_REALTIME, &stop);

    std::cout<< "Time to marshall int: "<<(stop.tv_sec - start.tv_sec)<<" s "
                                  <<(stop.tv_nsec - start.tv_nsec)<<" ns"<<std::endl;

    CustomResponse reply;
    ClientContext context;

    Status status = stub_->AcceptInt(&context, request, &reply);
    if (status.ok()) 
    {
      return reply.val();
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string AcceptDouble(const std::double_t& req) 
  {
    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    DoubleRequest request;
    request.set_val(req);
    clock_gettime(CLOCK_REALTIME, &stop);

    std::cout<< "Time to marshall double: "<<(stop.tv_sec - start.tv_sec)<<" s "
                                  <<(stop.tv_nsec - start.tv_nsec)<<" ns"<<std::endl;


    CustomResponse reply;
    ClientContext context;

    Status status = stub_->AcceptDouble(&context, request, &reply);
    if (status.ok()) 
    {
      return reply.val();
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string AcceptString(const std::string& req) 
  {
    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    StringRequest request;
    request.set_val(req);
    clock_gettime(CLOCK_REALTIME, &stop);

    std::cout<< "Time to marshall string: "<<(stop.tv_sec - start.tv_sec)<<" s "
                                  <<(stop.tv_nsec - start.tv_nsec)<<" ns"<<std::endl;

    CustomResponse reply;
    ClientContext context;

    Status status = stub_->AcceptString(&context, request, &reply);
    if (status.ok()) 
    {
      return reply.val();
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string AcceptComplexDataStructure(const std::int32_t& req1, const std::double_t& req2, const std::string& req3) 
  {
    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);
    ComplexDataStructureRequest request;
    request.set_val1(req1);
    request.set_val2(req2);
    request.set_val3(req3);
    clock_gettime(CLOCK_REALTIME, &stop);

    std::cout<< "Time to marshall complex: "<<(stop.tv_sec - start.tv_sec)<<" s "
                                  <<(stop.tv_nsec - start.tv_nsec)<<" ns"<<std::endl;

    CustomResponse reply;
    ClientContext context;

    Status status = stub_->AcceptComplexDataStructure(&context, request, &reply);
    if (status.ok()) 
    {
      return reply.val();
    } 
    else 
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string CreateString(std::int32_t size)
  {
    std::string ret = "";
    for (int i = 0; i < size; i++)
    {
      ret += "a";
    }

    return ret;
  }

  void AcceptClientSideStream() 
  {
    StringRequest request;
    CustomResponse reply;
    ClientContext context;
    const int kPoints = 16; // TO CHANGE

    std::unique_ptr<ClientWriter<StringRequest> > writer(
        stub_->AcceptClientSideStream(&context, &reply));
    for (int i = 0; i < kPoints; i++) 
    {
      request.set_val("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
      if (!writer->Write(request)) 
      {
        // Broken stream.
        break;
      }
    }
    writer->WritesDone();

    Status status = writer->Finish();
    if (status.ok()) 
    {
      std::cout << "Finished" << std::endl;
    } 
    else 
    {
      std::cout << "AcceptClientSideStream rpc failed." << std::endl;
    }
  }


 private:
  std::unique_ptr<Greeter::Stub> stub_;
  
};

int main(int argc, char** argv) 
{
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  std::string target_str;
  target_str = "localhost:50051";
  GreeterClient greeter(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  std::string reply;

  if (argc < 2)
  {
    std::cout << "Failed: Need arguments to run" << std::endl;
    return 1;
  }

  // TODO: Add desc of argv[1] types
  int argIntVal = std::stoi(argv[1]);
  switch (argIntVal)
  {
    // Time to marshall int
    case 1:
    {
      std::int32_t intReq = 1;
      reply = greeter.AcceptInt(intReq);
      //std::cout << "Greeter received: " << reply << std::endl;
      break;
    }
    
    // Time to marshall double
    case 2:
    { 
      std::double_t doubleReq = 1.0;
      reply = greeter.AcceptDouble(doubleReq);
      //std::cout << "Greeter received: " << reply << std::endl;
      break;
    }
    
    // Time to marshall string
    case 3:
    {
      
      if (argc != 3) 
      {
        std::cout << "Failure: Specify len" <<std::endl;
      }
      std::string stringReq = greeter.CreateString(std::stoi(argv[2]));
      reply = greeter.AcceptString(stringReq);
      //std::cout << "Greeter received: " << reply << std::endl;
      break;
    }

    // Time to marshall complex ds
    case 4: 
    {
      std::int32_t intReq = 1;
      std::double_t doubleReq = 1.0;
      std::string stringReq = "Hi";
      reply = greeter.AcceptComplexDataStructure(intReq, doubleReq, stringReq);
      //std::cout << "Greeter received: " << reply << std::endl;
      break;
    }

    // round trip time for small message
    // measure request + response time
    // run multiples time to see if first request takes longer than subsequent requests
    case 5: 
    {
      struct timespec start, stop;
      int iterations = 10;
      for (int i = 0; i < iterations; i++)
      {
        std::int32_t intReq = 1;
        clock_gettime(CLOCK_REALTIME, &start);
        reply = greeter.AcceptInt(intReq);
        clock_gettime(CLOCK_REALTIME, &stop);
        //std::cout << "Greeter received: " << reply << std::endl;
        std::cout<< "Round trip time to send small message: "<<(stop.tv_sec - start.tv_sec)<<" s "
                                  <<(stop.tv_nsec - start.tv_nsec)<<" ns"<<std::endl;
      }
      break;
    }

    // TODO: try for multiple sizes
    case 6:
    {
      int stringSizes[] = {512, 1024, 1536, 2048, 2560, 3072, 3584, 4096, 4608, 5120};
      std::string req = greeter.CreateString(stringSizes[1]);
      struct timespec start, stop;

      clock_gettime(CLOCK_REALTIME, &start);
      reply = greeter.AcceptString(req);
      clock_gettime(CLOCK_REALTIME, &stop);

      std::cout<< "Round trip time to send large message of size " << stringSizes[1] << ": "<<(stop.tv_sec - start.tv_sec)<<" s "
                                  <<(stop.tv_nsec - start.tv_nsec)<<" ns"<<std::endl;

      break;
    }

     // TODO: large message w client streaming
    case 7:
    {
      struct timespec start, stop;

      clock_gettime(CLOCK_REALTIME, &start);
      greeter.AcceptClientSideStream();
      clock_gettime(CLOCK_REALTIME, &stop);

      std::cout<< "Round trip time to send large message client streaming of size " << 1024 << ": " <<(stop.tv_sec - start.tv_sec)<< " s "
                                  << (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

      break;
    }

     // TODO: large message w server streaming
    case 8:
    {
      break;
    }

    default:
    {
      std::cout << "No such arg" << std::endl;
      break;
    }
  }

  return 0;
}
