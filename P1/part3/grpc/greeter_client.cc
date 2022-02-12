#include <iostream>
#include <memory>
#include <string>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

#define ITERATIONS 100
#define CLOCK_TYPE CLOCK_MONOTONIC

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
    IntRequest request;
    request.set_val(req);

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
    DoubleRequest request;
    request.set_val(req);

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
    StringRequest request;
    request.set_val(req);

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
    ComplexDataStructureRequest request;
    request.set_val1(req1);
    request.set_val2(req2);
    request.set_val3(req3);

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

  // dividing into fixed sized chunks
  void AcceptClientSideStream1(std::int32_t size) 
  {
    StringRequest request;
    CustomResponse reply;
    ClientContext context;
    int chunkSize = 128;
    int iterations = size / chunkSize;
    int remainder = size % chunkSize;
    if (size < chunkSize)
    {
      chunkSize = size;
    }
    std::string toSend = CreateString(chunkSize);

    std::unique_ptr<ClientWriter<StringRequest> > writer(
        stub_->AcceptClientSideStream(&context, &reply));
    for (int i = 0; i < iterations; i++) 
    {
      request.set_val(toSend);
      if (!writer->Write(request)) 
      {
        // Broken stream.
        std::cout << "AcceptClientSideStream broke" << std::endl;
        break;
      }
    }

    if (remainder)
    {
      request.set_val(CreateString(remainder));
      // Broken stream.
      if (!writer->Write(request)) 
      {
        std::cout << "AcceptClientSideStream broke" << std::endl;
      }
    }
    writer->WritesDone();

    Status status = writer->Finish();
    if (status.ok()) 
    {
      // std::cout << "Finished" << std::endl;
    } 
    else 
    {
      // std::cout << "AcceptClientSideStream rpc failed." << std::endl;
    }
  }

  // dividing into fixed stream iterations
  void AcceptClientSideStream2(std::int32_t size) 
  {
    StringRequest request;
    CustomResponse reply;
    ClientContext context;
    int streamIterations = 8;
    
    std::string toSend = CreateString(size / streamIterations);

    std::unique_ptr<ClientWriter<StringRequest> > writer(
        stub_->AcceptClientSideStream(&context, &reply));
    for (int i = 0; i < streamIterations; i++) 
    {
      request.set_val(toSend);
      if (!writer->Write(request)) 
      {
        // Broken stream.
        std::cout << "AcceptClientSideStream broke" << std::endl;
        break;
      }
    }

    writer->WritesDone();

    Status status = writer->Finish();
    if (status.ok()) 
    {
      // std::cout << "Finished" << std::endl;
    } 
    else 
    {
      // std::cout << "AcceptClientSideStream rpc failed." << std::endl;
    }
  }

  void showFlagUsage()
  {
    std::cout << "Usage: -t <num> to run test cases" << std::endl;
    std::cout << "where num is" << std::endl;
    std::cout << "1 = Marshal & Unmarshall Int" << std::endl;
    std::cout << "2 = Marshal & Unmarshal Double" << std::endl;
    std::cout << "3 = Marshal & Unmarshal Strings" << std::endl;
    std::cout << "4 = Marshal & Unmarshal Complex Data Structures" << std::endl;
    std::cout << "5 = Round trip time of small messages" << std::endl;
    std::cout << "6 = Round trip time of large messages (without streaming)" << std::endl;
    std::cout << "7 = Round trip time of large messages (with client side streaming 1)" << std::endl;
    std::cout << "7 = Round trip time of large messages (with client side streaming 2)" << std::endl;
  }

 private:
  std::unique_ptr<Greeter::Stub> stub_;
  
};

int main(int argc, char** argv) 
{
  opterr = 0;
  std::string target_str;
  target_str = "localhost:50051";
  //target_str = "52.151.53.152:50051";
  GreeterClient greeter(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  std::string reply;
  std::int32_t tFlagValue = 0;
  std::int32_t hflag = 0;
  std::int32_t c;
  while ((c = getopt(argc, argv, "ht:")) != -1)
  {
    switch (c)
    {
      case 'h':
        hflag = 1;
        break;
      case 't':
        tFlagValue = std::stoi(optarg);
        break;
      case '?':
        if (optopt == 'c')
        {
          std::cout << "Option -" << optopt << "requires an argument."<< std::endl;
        }
        else if (isprint (optopt))
        {
          std::cout << "Unknown option " << optopt << std::endl;
        }
        else
        {  
          std::cout << "Unknown option character" << optopt << std::endl;
        }
      default:
        abort();

    }
  }

  if (hflag)
  {
    greeter.showFlagUsage();
  }

  if (tFlagValue == 0)
  {
    return 0;
  }

  int argIntVal = tFlagValue;
  switch (argIntVal)
  {
    // Time to marshall int
    case 1:
    {
      std::cout<< " *** TESTING INT *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::int32_t intReq = 1;
        reply = greeter.AcceptInt(intReq);
      }
      
      //std::cout << "Greeter received: " << reply << std::endl;
      break;
    }
    
    // Time to marshall double
    case 2:
    { 
      std::cout<< " *** TESTING DOUBLE *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::double_t doubleReq = 1.0;
        reply = greeter.AcceptDouble(doubleReq);
      }
      
      //std::cout << "Greeter received: " << reply << std::endl;
      break;
    }
    
    // Time to marshall string
    case 3:
    {
      int32_t len1 = 512;
      int32_t len2 = 1024;
      int32_t len3 = 2048;

      std::cout<< " *** TESTING STRING OF LEN = 512 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::string stringReq = greeter.CreateString(len1);
        reply = greeter.AcceptString(stringReq);
      }

      std::cout<< " *** TESTING STRING OF LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::string stringReq = greeter.CreateString(len2);
        reply = greeter.AcceptString(stringReq);
      }

      std::cout<< " *** TESTING STRING OF LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::string stringReq = greeter.CreateString(len3);
        reply = greeter.AcceptString(stringReq);
      }

      //std::cout << "Greeter received: " << reply << std::endl;
      break;
    }

    // Time to marshall complex ds
    case 4: 
    {
      int32_t len1 = 512;
      int32_t len2 = 1024;
      int32_t len3 = 2048;

      std::cout<< " *** TESTING COMPLEX DATA STRUCTURE 1 *** " << std::endl;
      std::cout<< " *** CONTENTS: INT, DOUBLE, STRING LEN = 512 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = greeter.CreateString(len1);
        reply = greeter.AcceptComplexDataStructure(intReq, doubleReq, stringReq);
      }

      std::cout<< " *** TESTING COMPLEX DATA STRUCTURE 2 *** " << std::endl;
      std::cout<< " *** CONTENTS: INT, DOUBLE, STRING LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = greeter.CreateString(len2);
        reply = greeter.AcceptComplexDataStructure(intReq, doubleReq, stringReq);
      }

      std::cout<< " *** TESTING COMPLEX DATA STRUCTURE 3 *** " << std::endl;
      std::cout<< " *** CONTENTS: INT, DOUBLE, STRING LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = greeter.CreateString(len3);
        reply = greeter.AcceptComplexDataStructure(intReq, doubleReq, stringReq);
      }
      //std::cout << "Greeter received: " << reply << std::endl;
      break;
    }

    // round trip time for small message
    // measure request + response time
    // run multiples time to see if first request takes longer than subsequent requests
    case 5: 
    {
      struct timespec start, stop;
      int32_t len1 = 512;
      int32_t len2 = 1024;
      int32_t len3 = 2048;

      std::cout<< " *** TESTING INT *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::int32_t intReq = 1;
        reply = greeter.AcceptInt(intReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING DOUBLE *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::double_t doubleReq = 1.0;
        reply = greeter.AcceptDouble(doubleReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING STRING OF LEN = 512 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::string stringReq = greeter.CreateString(len1);
        reply = greeter.AcceptString(stringReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING STRING OF LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::string stringReq = greeter.CreateString(len2);
        reply = greeter.AcceptString(stringReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING STRING OF LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::string stringReq = greeter.CreateString(len3);
        reply = greeter.AcceptString(stringReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING COMPLEX DATA STRUCTURE 1 *** " << std::endl;
      std::cout<< " *** CONTENTS: INT, DOUBLE, STRING LEN = 512 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = greeter.CreateString(len1);
        reply = greeter.AcceptComplexDataStructure(intReq, doubleReq, stringReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING COMPLEX DATA STRUCTURE 1 *** " << std::endl;
      std::cout<< " *** CONTENTS: INT, DOUBLE, STRING LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = greeter.CreateString(len2);
        reply = greeter.AcceptComplexDataStructure(intReq, doubleReq, stringReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING COMPLEX DATA STRUCTURE 1 *** " << std::endl;
      std::cout<< " *** CONTENTS: INT, DOUBLE, STRING LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = greeter.CreateString(len3);
        reply = greeter.AcceptComplexDataStructure(intReq, doubleReq, stringReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }


      break;
    }

    // large messages without streaming
    case 6:
    {
      struct timespec start, stop;
      int sizeArr[] = {128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072};
      int len = sizeof(sizeArr)/sizeof(sizeArr[0]);

      for (int i = 0; i < len; i++)
      {
        std::cout<< " *** TESTING STRING OF LEN = " << sizeArr[i]  << " *** " << std::endl;
        for (int j = 0; j < ITERATIONS; j++)
        {
          clock_gettime(CLOCK_TYPE, &start);
          std::string req = greeter.CreateString(sizeArr[i]);
          reply = greeter.AcceptString(req);
          clock_gettime(CLOCK_TYPE, &stop);
          std::cout<< "Round trip time to send large message : " << (stop.tv_sec - start.tv_sec)<< " s "
                                  << (stop.tv_nsec - start.tv_nsec) << " ns"<<std::endl;
        }
      }
    
      break;
    }

    // large message w client streaming 1
    case 7:
    {
      struct timespec start, stop;
      int sizeArr[] = {128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072};
      int len = sizeof(sizeArr)/sizeof(sizeArr[0]);
      for (int i = 0; i < len; i++) 
      {
        std::cout<< " *** TESTING STRING OF LEN = " << sizeArr[i]  << " *** " << std::endl;
        for (int j = 0; j < ITERATIONS; j++) 
        {
          clock_gettime(CLOCK_TYPE, &start);
          greeter.AcceptClientSideStream1(sizeArr[i]);
          //greeter.AcceptClientSideStream2(sizeArr[i]);
          clock_gettime(CLOCK_TYPE, &stop);
          std::cout<< "Round trip time to send large message with client streaming 1 : " << (stop.tv_sec - start.tv_sec)<< " s "
                                  << (stop.tv_nsec - start.tv_nsec) << " ns"<<std::endl;
        }
        
      }

      break;
    }
    // large message w client streaming 2
    case 8:
    {
      struct timespec start, stop;
      int sizeArr[] = {128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072};
      int len = sizeof(sizeArr)/sizeof(sizeArr[0]);
      for (int i = 0; i < len; i++) 
      {
        std::cout<< " *** TESTING STRING OF LEN = " << sizeArr[i]  << " *** " << std::endl;
        for (int j = 0; j < ITERATIONS; j++) 
        {
          clock_gettime(CLOCK_TYPE, &start);
          //greeter.AcceptClientSideStream1(sizeArr[i]);
          greeter.AcceptClientSideStream2(sizeArr[i]);
          clock_gettime(CLOCK_TYPE, &stop);
          std::cout<< "Round trip time to send large message with client streaming 2 : " << (stop.tv_sec - start.tv_sec)<< " s "
                                  << (stop.tv_nsec - start.tv_nsec) << " ns"<<std::endl;
        }
        
      }

      break;
    }
    default:
    {
      std::cout << "Invalid argument for -t flag. Run -h to learn more" << std::endl;
      break;
    }
  }

  return 0;
}
