#include "gen-cpp/CustomSvc.h"
// #include "gen-cpp/rpc_measurement_types.h"
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <memory>
#include <iostream>
#include <string>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#define ITERATIONS 5
#define CLOCK_TYPE CLOCK_MONOTONIC

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using std::shared_ptr;
using std::make_shared;
using namespace std;

class CustomSvcHelperClass{
    public:

     std::string CreateString(std::int32_t size)
  {
    std::string ret = "";
    for (int i = 0; i < size; i++)
    {
      ret += "a";
    }

    return ret;
  }

    void showOptionUsage()
    {
    cout << "Usage: -t <num> to run test cases" << endl;
    cout << "where num is" << endl;
    cout << "1 = Testing Int" << endl;
    cout << "2 = Testing Double" << endl;
    cout << "3 = Testing Strings" << endl;
    cout << "4 = Testing Complex Data Structures" << endl;
    cout << "5 = Round trip time of small messages" << endl;
    cout << "6 = Round trip time of large messages (without streaming)" << endl;
    cout << "7 = Round trip time of large messages (with client side streaming)" << endl;

    }
};

  // struct ComplexDataStructure
  // {
  //   int32_t val1;
  //   double val2;
  //   string val3;
  // };

int main(int argc, char **argv)
{
    shared_ptr<TTransport> trans;
    trans = make_shared<TSocket>("localhost", 5050);
    trans = make_shared<TBufferedTransport>(trans);
    auto proto = make_shared<TBinaryProtocol>(trans);
    CustomSvcClient client(proto);

    opterr = 0;

    string reply_str;

    CustomSvcHelperClass custom_helper;

    // int32_t c;

    int32_t tFlagValue = 0;
  int32_t hflag = 0;
  int32_t c;
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
          cout << "Option -" << optopt << "requires an argument."<< endl;
        }
        else if (isprint (optopt))
        {
          cout << "Unknown option " << optopt << endl;
        }
        else
        {  
          cout << "Unknown option character" << optopt << endl;
        }
      default:
        abort();

    }
  }

  if (hflag)
  {
    custom_helper.showOptionUsage();
  }

  if (tFlagValue == 0)
  {
    return 0;
  }

  int argIntVal = tFlagValue;

try{
 trans->open();

  switch (argIntVal)
  {
    // Time to marshall int
    case 1:
    {
      std::cout<< " *** TESTING INT *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::int32_t intReq = 1;
        client.AcceptInt(reply_str, intReq);
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
        client.AcceptDouble(reply_str, doubleReq);
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
        std::string stringReq = custom_helper.CreateString(len1);
         client.AcceptString(reply_str, stringReq);
      }

      std::cout<< " *** TESTING STRING OF LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::string stringReq = custom_helper.CreateString(len2);
        client.AcceptString(reply_str, stringReq);
      }

      std::cout<< " *** TESTING STRING OF LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::string stringReq = custom_helper.CreateString(len3);
        client.AcceptString(reply_str, stringReq);
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
        std::string stringReq = custom_helper.CreateString(len1);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        client.AcceptComplexDataStructure(reply_str, complex_ds);
      }

      std::cout<< " *** TESTING COMPLEX DATA STRUCTURE 2 *** " << std::endl;
      std::cout<< " *** CONTENTS: INT, DOUBLE, STRING LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = custom_helper.CreateString(len2);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        client.AcceptComplexDataStructure(reply_str, complex_ds);
      }

      std::cout<< " *** TESTING COMPLEX DATA STRUCTURE 3 *** " << std::endl;
      std::cout<< " *** CONTENTS: INT, DOUBLE, STRING LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = custom_helper.CreateString(len3);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        client.AcceptComplexDataStructure(reply_str, complex_ds);
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
        client.AcceptInt(reply_str, intReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING DOUBLE *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::double_t doubleReq = 1.0;
        client.AcceptDouble(reply_str, doubleReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING STRING OF LEN = 512 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::string stringReq = custom_helper.CreateString(len1);
        client.AcceptString(reply_str, stringReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING STRING OF LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::string stringReq = custom_helper.CreateString(len2);
        client.AcceptString(reply_str, stringReq);
        clock_gettime(CLOCK_TYPE, &stop);
        std::cout<< "Round trip time to send small message: "<< (stop.tv_sec - start.tv_sec) <<" s "
                                  << (stop.tv_nsec - start.tv_nsec) <<" ns" << std::endl;
      }

      std::cout<< " *** TESTING STRING OF LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++)
      {
        clock_gettime(CLOCK_TYPE, &start);
        std::string stringReq = custom_helper.CreateString(len3);
        client.AcceptString(reply_str, stringReq);
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
        std::string stringReq = custom_helper.CreateString(len1);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        client.AcceptComplexDataStructure(reply_str, complex_ds);
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
        std::string stringReq = custom_helper.CreateString(len2);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        client.AcceptComplexDataStructure(reply_str, complex_ds);
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
        std::string stringReq = custom_helper.CreateString(len3);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        client.AcceptComplexDataStructure(reply_str, complex_ds);
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
          std::string req = custom_helper.CreateString(sizeArr[i]);
          client.AcceptString(reply_str, req);
          clock_gettime(CLOCK_TYPE, &stop);
          std::cout<< "Round trip time to send large message : " << (stop.tv_sec - start.tv_sec)<< " s "
                                  << (stop.tv_nsec - start.tv_nsec) << " ns"<<std::endl;
        }
      }
    
      break;
    }

    // large message w client streaming
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
          //greeter.AcceptClientSideStream1(sizeArr[i]);
          
          // todo to check and fix the streaming part of this

          //client.AcceptClientSideStream(reply_str, sizeArr[i]);
          clock_gettime(CLOCK_TYPE, &stop);
          std::cout<< "Round trip time to send large message with client streaming : " << (stop.tv_sec - start.tv_sec)<< " s "
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

}
catch(...) {
        std::cout << "Client caught an exception" << std::endl;
    }

   
  trans->close();
  return 0;
}




// int main() {
//     shared_ptr<TTransport> trans;
//     trans = make_shared<TSocket>("localhost", 9090);
//     trans = make_shared<TBufferedTransport>(trans);
//     auto proto = make_shared<TBinaryProtocol>(trans);
//     CustomSvcClient client(proto);

//     try {
//         trans->open();
//         std::string msg;
//         client.getStringMessage(msg, "test message");
//         std::cout << msg << std::endl;
//     } catch(...) {
//         std::cout << "Client caught an exception" << std::endl;
//     }
//     trans->close();
// }
