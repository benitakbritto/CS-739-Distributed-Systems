#include "gen-cpp/CustomSvc.h"
 // #include "gen-cpp/rpc_measurement_types.h"
#include <thrift/transport/TSocket.h>

#include <thrift/transport/TBufferTransports.h>

#include <thrift/protocol/TBinaryProtocol.h>

#include <thrift/protocol/TCompactProtocol.h>

#include <thrift/protocol/TJSONProtocol.h>

#include <memory>

#include <iostream>

#include <string>

#include <time.h>

#include <ctype.h>

#include <unistd.h>

#define ITERATIONS 100
#define CLOCK_TYPE CLOCK_MONOTONIC

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using std::shared_ptr;
using std::make_shared;
using namespace std;

class CustomSvcHelperClass {
  public:

    std::string CreateString(std::int32_t size) {
      std::string ret = "";
      for (int i = 0; i < size; i++) {
        ret += "a";
      }

      return ret;
    }

  void TestStringTime(string stringReq) {
    struct timespec start, stop;
    int32_t size_written;

    // serialisation - marshalling 
    // Transport protocol - can change later 
    auto trans_m = make_shared < TMemoryBuffer > (4096);

    // Serialisation protocol (binary, compact, json) are present
    TBinaryProtocol binary_proto(trans_m);

    clock_gettime(CLOCK_TYPE, & start);

    size_written = binary_proto.writeString(stringReq);

    clock_gettime(CLOCK_TYPE, & stop);
    std::cout << "Time to marshall string of 512 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
      (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

    cout << "Wrote " << size_written << " bytes to TMemoryBuffer" << endl;

    // capturing time to marshall integer in TCompactProtocol     

    // serialisation - marshalling         

    // Serialisation protocol (binary, compact, json) are present
    TCompactProtocol compact_proto(trans_m);

    clock_gettime(CLOCK_TYPE, & start);

    size_written = compact_proto.writeString(stringReq);

    clock_gettime(CLOCK_TYPE, & stop);
    std::cout << "Time to marshall integer with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
      (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

    cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

  }

  void showOptionUsage() {
    cout << "Usage: -t <num> to run test cases" << endl;
    cout << "where num is" << endl;
    cout << "1 = Testing Int" << endl;
    cout << "2 = Testing Double" << endl;
    cout << "3 = Testing Strings" << endl;
    cout << "4 = Testing Complex Data Structures" << endl;
    cout << "5 = Round trip time of small messages" << endl;
    cout << "6 = Round trip time of large messages (without streaming)" << endl;
    cout << "7 = Round trip time of large messages (with client side streaming)" << endl;
        cout << "8 = Varying size BW check (10MB, 20MB etc)" << endl;

  }
};

int main(int argc, char ** argv) {
  shared_ptr < TTransport > trans;
  trans = make_shared < TSocket > ("localhost", 5050);
  trans = make_shared < TBufferedTransport > (trans);
  auto proto = make_shared < TBinaryProtocol > (trans);
  CustomSvcClient client(proto);

  opterr = 0;

  string reply_str;

  CustomSvcHelperClass custom_helper;

  // int32_t c;

  int32_t tFlagValue = 0;
  int32_t hflag = 0;
  int32_t c;
  while ((c = getopt(argc, argv, "ht:")) != -1) {
    switch (c) {
    case 'h':
      hflag = 1;
      break;
    case 't':
      tFlagValue = std::stoi(optarg);
      break;
    case '?':
      if (optopt == 'c') {
        cout << "Option -" << optopt << "requires an argument." << endl;
      } else if (isprint(optopt)) {
        cout << "Unknown option " << optopt << endl;
      } else {
        cout << "Unknown option character" << optopt << endl;
      }
      default:
        abort();

    }
  }

  if (hflag) {
    custom_helper.showOptionUsage();
  }

  if (tFlagValue == 0) {
    return 0;
  }

  int argIntVal = tFlagValue;

  try {
    trans -> open();

    switch (argIntVal) {
      // Time to marshall int
    case 1: {
      struct timespec start, stop;
      std::cout << " *** TESTING INT *** " << std::endl;
      double binm_av_time = 0, compactm_av_time = 0;
      double binm_total_time = 0, compactm_total_time = 0;
      double binu_av_time = 0, compactu_av_time = 0;
      double binu_total_time = 0, compactu_total_time = 0;
      for (int i = 0; i < ITERATIONS; i++) {

        std::int32_t intReq = 1;

        int32_t size_written;

        // serialisation - marshalling 
        // Transport protocol - can change later 
        auto trans_m = make_shared < TMemoryBuffer > (4096);

        // Serialisation protocol (binary, compact, json) are present
        TBinaryProtocol binary_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.writeI32(intReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        binm_total_time += (stop.tv_nsec - start.tv_nsec);
        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        // unmarshal
        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.readI32(intReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to umarshall integer with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        binu_total_time += (stop.tv_nsec - start.tv_nsec);

        // capturing time to marshall integer in TCompactProtocol

        // Serialisation protocol (binary, compact, json) are present
        TCompactProtocol compact_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.writeI32(intReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        compactm_total_time += (stop.tv_nsec - start.tv_nsec);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.readI32(intReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to umarshall integer with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        compactu_total_time += (stop.tv_nsec - start.tv_nsec);

      }
      binm_av_time = binm_total_time / ITERATIONS;
      compactm_av_time = compactm_total_time / ITERATIONS;

      binu_av_time = binu_total_time / ITERATIONS;
      compactu_av_time = compactu_total_time / ITERATIONS;

      cout << "Average Marshall Int time: Binary-> " << binm_av_time << " Compact-> " << compactm_av_time << endl;

      cout << "Average Umarshall Int time: binary-> " << binu_av_time << " Compact -> " << compactu_av_time << endl;

      break;
    }

    // Time to marshall double
    case 2: {
      struct timespec start, stop;
      std::cout << " *** TESTING DOUBLE *** " << std::endl;
      double binm_av_time = 0, compactm_av_time = 0;
      double binm_total_time = 0, compactm_total_time = 0;
      double binu_av_time = 0, compactu_av_time = 0;
      double binu_total_time = 0, compactu_total_time = 0;
      for (int i = 0; i < ITERATIONS; i++) {

        std::double_t doubleReq = 1;

        int32_t size_written;

        // serialisation - marshalling 
        // Transport protocol - can change later 
        auto trans_m = make_shared < TMemoryBuffer > (4096);

        // Serialisation protocol (binary, compact, json) are present
        TBinaryProtocol binary_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.writeDouble(doubleReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall double with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        binm_total_time += (stop.tv_nsec - start.tv_nsec);
        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        // unmarshal
        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.readDouble(doubleReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to umarshall double with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        binu_total_time += (stop.tv_nsec - start.tv_nsec);

        TCompactProtocol compact_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.writeDouble(doubleReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        compactm_total_time += (stop.tv_nsec - start.tv_nsec);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.readDouble(doubleReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to umarshall integer with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        compactu_total_time += (stop.tv_nsec - start.tv_nsec);

      }
      binm_av_time = binm_total_time / ITERATIONS;
      compactm_av_time = compactm_total_time / ITERATIONS;

      binu_av_time = binu_total_time / ITERATIONS;
      compactu_av_time = compactu_total_time / ITERATIONS;

      cout << "Average Marshall Double time: Binary-> " << binm_av_time << " Compact-> " << compactm_av_time << endl;

      cout << "Average Umarshall Double time: binary-> " << binu_av_time << " Compact -> " << compactu_av_time << endl;

      break;
    }

    // Time to marshall string
    case 3: {

      struct timespec start, stop;
      int32_t len1 = 512;
      int32_t len2 = 1024;
      int32_t len3 = 2048;

      double binm_av_time = 0, compactm_av_time = 0;
      double binm_total_time = 0, compactm_total_time = 0;
      double binu_av_time = 0, compactu_av_time = 0;
      double binu_total_time = 0, compactu_total_time = 0;

      std::cout << " *** TESTING STRING OF LEN = 512 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        std::string stringReq = custom_helper.CreateString(len1);

        int32_t size_written;

        // serialisation - marshalling 
        // Transport protocol - can change later 
        auto trans_m = make_shared < TMemoryBuffer > (4096);

        // Serialisation protocol (binary, compact, json) are present
        TBinaryProtocol binary_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.writeString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall string of 512 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        binm_total_time += (stop.tv_nsec - start.tv_nsec);
        cout << "Wrote " << size_written << " bytes to TMemoryBuffer" << endl;

        // unmarshall

        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.readString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall string of 512 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        binu_total_time += (stop.tv_nsec - start.tv_nsec);

        // capturing time to marshall integer in TCompactProtocol

        // Serialisation protocol (binary, compact, json) are present
        TCompactProtocol compact_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.writeString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        compactm_total_time += (stop.tv_nsec - start.tv_nsec);
        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.readString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        compactu_total_time += (stop.tv_nsec - start.tv_nsec);

      }

      binm_av_time = binm_total_time / ITERATIONS;
      compactm_av_time = compactm_total_time / ITERATIONS;

      binu_av_time = binu_total_time / ITERATIONS;
      compactu_av_time = compactu_total_time / ITERATIONS;

      cout << "Average Marshall String len1: Binary-> " << binm_av_time << " Compact-> " << compactm_av_time << endl;

      cout << "Average Umarshall string len1: binary-> " << binu_av_time << " Compact -> " << compactu_av_time << endl;

      binm_av_time = 0, compactm_av_time = 0;
      binm_total_time = 0, compactm_total_time = 0;
      binu_av_time = 0, compactu_av_time = 0;
      binu_total_time = 0, compactu_total_time = 0;

      std::cout << " *** TESTING STRING OF LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        std::string stringReq = custom_helper.CreateString(len2);

        int32_t size_written;

        // serialisation - marshalling 
        // Transport protocol - can change later 
        auto trans_m = make_shared < TMemoryBuffer > (4096);

        // Serialisation protocol (binary, compact, json) are present
        TBinaryProtocol binary_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.writeString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall string of 1024 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        binm_total_time += (stop.tv_nsec - start.tv_nsec);
        cout << "Wrote " << size_written << " bytes to TMemoryBuffer" << endl;

        // unmarshall

        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.readString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall string of 1024 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        binu_total_time += (stop.tv_nsec - start.tv_nsec);

        // capturing time to marshall integer in TCompactProtocol

        // Serialisation protocol (binary, compact, json) are present
        TCompactProtocol compact_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.writeString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall  with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        compactm_total_time += (stop.tv_nsec - start.tv_nsec);
        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.readString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall  with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        compactu_total_time += (stop.tv_nsec - start.tv_nsec);

      }

      binm_av_time = binm_total_time / ITERATIONS;
      compactm_av_time = compactm_total_time / ITERATIONS;

      binu_av_time = binu_total_time / ITERATIONS;
      compactu_av_time = compactu_total_time / ITERATIONS;

      cout << "Average Marshall String len2: Binary-> " << binm_av_time << " Compact-> " << compactm_av_time << endl;

      cout << "Average Umarshall string len2: binary-> " << binu_av_time << " Compact -> " << compactu_av_time << endl;

      //////

      binm_av_time = 0, compactm_av_time = 0;
      binm_total_time = 0, compactm_total_time = 0;
      binu_av_time = 0, compactu_av_time = 0;
      binu_total_time = 0, compactu_total_time = 0;

      std::cout << " *** TESTING STRING OF LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        std::string stringReq = custom_helper.CreateString(len3);

        int32_t size_written;

        // serialisation - marshalling 
        // Transport protocol - can change later 
        auto trans_m = make_shared < TMemoryBuffer > (16384);

        // Serialisation protocol (binary, compact, json) are present
        TBinaryProtocol binary_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.writeString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall string of 2048 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        binm_total_time += (stop.tv_nsec - start.tv_nsec);
        cout << "Wrote " << size_written << " bytes to TMemoryBuffer" << endl;

        // unmarshall

        clock_gettime(CLOCK_TYPE, & start);

        size_written = binary_proto.readString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall string of 2048 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        binu_total_time += (stop.tv_nsec - start.tv_nsec);

        // capturing time to marshall integer in TCompactProtocol

        // Serialisation protocol (binary, compact, json) are present
        TCompactProtocol compact_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.writeString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall  with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        compactm_total_time += (stop.tv_nsec - start.tv_nsec);
        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        clock_gettime(CLOCK_TYPE, & start);

        size_written = compact_proto.readString(stringReq);

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall  with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        compactu_total_time += (stop.tv_nsec - start.tv_nsec);

      }

      binm_av_time = binm_total_time / ITERATIONS;
      compactm_av_time = compactm_total_time / ITERATIONS;

      binu_av_time = binu_total_time / ITERATIONS;
      compactu_av_time = compactu_total_time / ITERATIONS;

      cout << "Average Marshall String len3: Binary-> " << binm_av_time << " Compact-> " << compactm_av_time << endl;

      cout << "Average Umarshall string len3: binary-> " << binu_av_time << " Compact -> " << compactu_av_time << endl;

      break;
    }

    // Time to marshall complex ds
    case 4: {
      struct timespec start, stop;
      int32_t len1 = 512;
      int32_t len2 = 1024;
      int32_t len3 = 2048;

      double binm_av_time = 0, compactm_av_time = 0;
      double binm_total_time = 0, compactm_total_time = 0;
      double binu_av_time = 0, compactu_av_time = 0;
      double binu_total_time = 0, compactu_total_time = 0;

      std::cout << " *** TESTING COMPLEX DATA STRUCTURE 1 *** " << std::endl;
      std::cout << " *** CONTENTS: INT, DOUBLE, STRING LEN = 512 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {

        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = custom_helper.CreateString(len1);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        //client.AcceptComplexDataStructure(reply_str, complex_ds);

        int32_t size_written;

        // serialisation - marshalling 
        // Transport protocol - can change later 
        auto trans_m = make_shared < TMemoryBuffer > (4096);

        // Serialisation protocol (binary, compact, json) are present
        TBinaryProtocol binary_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written += binary_proto.writeStructBegin("ComplexDataStructure");
        size_written += binary_proto.writeFieldBegin("val1", ::apache::thrift::protocol::T_I32, 1);
        size_written += binary_proto.writeI32(intReq);
        size_written += binary_proto.writeFieldEnd();

        size_written += binary_proto.writeFieldBegin("val2", ::apache::thrift::protocol::T_DOUBLE, 2);
        size_written += binary_proto.writeDouble(doubleReq);
        size_written += binary_proto.writeFieldEnd();

        size_written += binary_proto.writeFieldBegin("val3", ::apache::thrift::protocol::T_STRING, 3);
        size_written += binary_proto.writeString(stringReq);
        size_written += binary_proto.writeFieldEnd();

        size_written += binary_proto.writeFieldStop();
        size_written += binary_proto.writeStructEnd();

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall string of 512 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        binm_total_time += (stop.tv_nsec - start.tv_nsec);
        // capturing time to marshall integer in TCompactProtocol

        TCompactProtocol compact_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = 0;

        size_written += compact_proto.writeStructBegin("ComplexDataStructure");
        size_written += compact_proto.writeFieldBegin("val1", ::apache::thrift::protocol::T_I32, 1);
        size_written += compact_proto.writeI32(intReq);
        size_written += compact_proto.writeFieldEnd();

        size_written += compact_proto.writeFieldBegin("val2", ::apache::thrift::protocol::T_DOUBLE, 2);
        size_written += compact_proto.writeDouble(doubleReq);
        size_written += compact_proto.writeFieldEnd();

        size_written += compact_proto.writeFieldBegin("val3", ::apache::thrift::protocol::T_STRING, 3);
        size_written += compact_proto.writeString(stringReq);
        size_written += compact_proto.writeFieldEnd();

        size_written += compact_proto.writeFieldStop();
        size_written += compact_proto.writeStructEnd();

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        compactm_total_time += (stop.tv_nsec - start.tv_nsec);

        // capturing time to marshall integer in TJsonProtocol

        // Serialisation protocol (binary, compact, json) are present
        TJSONProtocol json_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = 0;

        size_written += json_proto.writeStructBegin("ComplexDataStructure");
        size_written += json_proto.writeFieldBegin("val1", ::apache::thrift::protocol::T_I32, 1);
        size_written += json_proto.writeI32(intReq);
        size_written += json_proto.writeFieldEnd();

        size_written += json_proto.writeFieldBegin("val2", ::apache::thrift::protocol::T_DOUBLE, 2);
        size_written += json_proto.writeDouble(doubleReq);
        size_written += json_proto.writeFieldEnd();

        size_written += json_proto.writeFieldBegin("val3", ::apache::thrift::protocol::T_STRING, 3);
        size_written += json_proto.writeString(stringReq);
        size_written += json_proto.writeFieldEnd();

        size_written += json_proto.writeFieldStop();
        size_written += json_proto.writeStructEnd();

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TJsonProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

      }

      binm_av_time = binm_total_time / ITERATIONS;
      compactm_av_time = compactm_total_time / ITERATIONS;

      binu_av_time = binu_total_time / ITERATIONS;
      compactu_av_time = compactu_total_time / ITERATIONS;

      cout << "Average Marshall complex len1: Binary-> " << binm_av_time << " Compact-> " << compactm_av_time << endl;

      cout << "Average Umarshall complex len1: binary-> " << binm_av_time << " Compact -> " << compactm_av_time << endl;

      binm_av_time = 0, compactm_av_time = 0;
      binm_total_time = 0, compactm_total_time = 0;
      binu_av_time = 0, compactu_av_time = 0;
      binu_total_time = 0, compactu_total_time = 0;

      std::cout << " *** TESTING COMPLEX DATA STRUCTURE 2 *** " << std::endl;
      std::cout << " *** CONTENTS: INT, DOUBLE, STRING LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {

        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = custom_helper.CreateString(len2);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        //client.AcceptComplexDataStructure(reply_str, complex_ds);

        int32_t size_written;

        // serialisation - marshalling 
        // Transport protocol - can change later 
        auto trans_m = make_shared < TMemoryBuffer > (4096);

        // Serialisation protocol (binary, compact, json) are present
        TBinaryProtocol binary_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written += binary_proto.writeStructBegin("ComplexDataStructure");
        size_written += binary_proto.writeFieldBegin("val1", ::apache::thrift::protocol::T_I32, 1);
        size_written += binary_proto.writeI32(intReq);
        size_written += binary_proto.writeFieldEnd();

        size_written += binary_proto.writeFieldBegin("val2", ::apache::thrift::protocol::T_DOUBLE, 2);
        size_written += binary_proto.writeDouble(doubleReq);
        size_written += binary_proto.writeFieldEnd();

        size_written += binary_proto.writeFieldBegin("val3", ::apache::thrift::protocol::T_STRING, 3);
        size_written += binary_proto.writeString(stringReq);
        size_written += binary_proto.writeFieldEnd();

        size_written += binary_proto.writeFieldStop();
        size_written += binary_proto.writeStructEnd();

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall string of 512 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        binm_total_time += (stop.tv_nsec - start.tv_nsec);



        string struct_name; 
        string field_name;
        TType field_type;
        int16_t field_id;

        clock_gettime(CLOCK_TYPE, & start);

        binary_proto.readStructBegin(struct_name);

        binary_proto.readFieldBegin(field_name, field_type, field_id);
        binary_proto.readI32(complex_ds.val1);
        binary_proto.readFieldEnd();

        binary_proto.readFieldBegin(field_name, field_type, field_id);
        binary_proto.readDouble(complex_ds.val2);
        binary_proto.readFieldEnd();

        binary_proto.readFieldBegin(field_name, field_type, field_id);
        binary_proto.readString(complex_ds.val3);
        binary_proto.readFieldEnd();
        
        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to unmarshall complex ds of 512 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        binu_total_time += (stop.tv_nsec - start.tv_nsec);

        // capturing time to marshall integer in TCompactProtocol

        // Serialisation protocol (binary, compact, json) are present
        TCompactProtocol compact_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = 0;

        size_written += compact_proto.writeStructBegin("ComplexDataStructure");
        size_written += compact_proto.writeFieldBegin("val1", ::apache::thrift::protocol::T_I32, 1);
        size_written += compact_proto.writeI32(intReq);
        size_written += compact_proto.writeFieldEnd();

        size_written += compact_proto.writeFieldBegin("val2", ::apache::thrift::protocol::T_DOUBLE, 2);
        size_written += compact_proto.writeDouble(doubleReq);
        size_written += compact_proto.writeFieldEnd();

        size_written += compact_proto.writeFieldBegin("val3", ::apache::thrift::protocol::T_STRING, 3);
        size_written += compact_proto.writeString(stringReq);
        size_written += compact_proto.writeFieldEnd();

        size_written += compact_proto.writeFieldStop();
        size_written += compact_proto.writeStructEnd();

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        compactm_total_time += (stop.tv_nsec - start.tv_nsec);

        // capturing time to marshall integer in TJsonProtocol

        // Serialisation protocol (binary, compact, json) are present
        TJSONProtocol json_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = 0;

        size_written += json_proto.writeStructBegin("ComplexDataStructure");
        size_written += json_proto.writeFieldBegin("val1", ::apache::thrift::protocol::T_I32, 1);
        size_written += json_proto.writeI32(intReq);
        size_written += json_proto.writeFieldEnd();

        size_written += json_proto.writeFieldBegin("val2", ::apache::thrift::protocol::T_DOUBLE, 2);
        size_written += json_proto.writeDouble(doubleReq);
        size_written += json_proto.writeFieldEnd();

        size_written += json_proto.writeFieldBegin("val3", ::apache::thrift::protocol::T_STRING, 3);
        size_written += json_proto.writeString(stringReq);
        size_written += json_proto.writeFieldEnd();

        size_written += json_proto.writeFieldStop();
        size_written += json_proto.writeStructEnd();

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TJsonProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

      }

      binm_av_time = binm_total_time / ITERATIONS;
      compactm_av_time = compactm_total_time / ITERATIONS;

      binu_av_time = binu_total_time / ITERATIONS;
      compactu_av_time = compactu_total_time / ITERATIONS;

      cout << "Average Marshall complex ds len2: Binary-> " << binm_av_time << " Compact-> " << compactm_av_time << endl;

      cout << "Average Umarshall complex ds len2: binary-> " << binu_av_time << " Compact -> " << compactu_av_time << endl;

      binm_av_time = 0, compactm_av_time = 0;
      binm_total_time = 0, compactm_total_time = 0;
      binu_av_time = 0, compactu_av_time = 0;
      binu_total_time = 0, compactu_total_time = 0;

      std::cout << " *** TESTING COMPLEX DATA STRUCTURE 3 *** " << std::endl;
      std::cout << " *** CONTENTS: INT, DOUBLE, STRING LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {

        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = custom_helper.CreateString(len3);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        //client.AcceptComplexDataStructure(reply_str, complex_ds);

        int32_t size_written;

        // serialisation - marshalling 
        // Transport protocol - can change later 
        auto trans_m = make_shared < TMemoryBuffer > (4096);

        // Serialisation protocol (binary, compact, json) are present
        TBinaryProtocol binary_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written += binary_proto.writeStructBegin("ComplexDataStructure");
        size_written += binary_proto.writeFieldBegin("val1", ::apache::thrift::protocol::T_I32, 1);
        size_written += binary_proto.writeI32(intReq);
        size_written += binary_proto.writeFieldEnd();

        size_written += binary_proto.writeFieldBegin("val2", ::apache::thrift::protocol::T_DOUBLE, 2);
        size_written += binary_proto.writeDouble(doubleReq);
        size_written += binary_proto.writeFieldEnd();

        size_written += binary_proto.writeFieldBegin("val3", ::apache::thrift::protocol::T_STRING, 3);
        size_written += binary_proto.writeString(stringReq);
        size_written += binary_proto.writeFieldEnd();

        size_written += binary_proto.writeFieldStop();
        size_written += binary_proto.writeStructEnd();

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall string of 512 chars with TBinaryProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        binm_total_time += (stop.tv_nsec - start.tv_nsec);
        // capturing time to marshall integer in TCompactProtocol

        // Serialisation protocol (binary, compact, json) are present
        TCompactProtocol compact_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = 0;

        size_written += compact_proto.writeStructBegin("ComplexDataStructure");
        size_written += compact_proto.writeFieldBegin("val1", ::apache::thrift::protocol::T_I32, 1);
        size_written += compact_proto.writeI32(intReq);
        size_written += compact_proto.writeFieldEnd();

        size_written += compact_proto.writeFieldBegin("val2", ::apache::thrift::protocol::T_DOUBLE, 2);
        size_written += compact_proto.writeDouble(doubleReq);
        size_written += compact_proto.writeFieldEnd();

        size_written += compact_proto.writeFieldBegin("val3", ::apache::thrift::protocol::T_STRING, 3);
        size_written += compact_proto.writeString(stringReq);
        size_written += compact_proto.writeFieldEnd();

        size_written += compact_proto.writeFieldStop();
        size_written += compact_proto.writeStructEnd();

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TCompactProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

        compactm_total_time += (stop.tv_nsec - start.tv_nsec);

        // capturing time to marshall integer in TJsonProtocol

        // Serialisation protocol (binary, compact, json) are present
        TJSONProtocol json_proto(trans_m);

        clock_gettime(CLOCK_TYPE, & start);

        size_written = 0;

        size_written += json_proto.writeStructBegin("ComplexDataStructure");
        size_written += json_proto.writeFieldBegin("val1", ::apache::thrift::protocol::T_I32, 1);
        size_written += json_proto.writeI32(intReq);
        size_written += json_proto.writeFieldEnd();

        size_written += json_proto.writeFieldBegin("val2", ::apache::thrift::protocol::T_DOUBLE, 2);
        size_written += json_proto.writeDouble(doubleReq);
        size_written += json_proto.writeFieldEnd();

        size_written += json_proto.writeFieldBegin("val3", ::apache::thrift::protocol::T_STRING, 3);
        size_written += json_proto.writeString(stringReq);
        size_written += json_proto.writeFieldEnd();

        size_written += json_proto.writeFieldStop();
        size_written += json_proto.writeStructEnd();

        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Time to marshall integer with TJsonProtocol: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;

        cout << "Wrote " << size_written << "bytes to TMemoryBuffer" << endl;

      }

      binm_av_time = binm_total_time / ITERATIONS;
      compactm_av_time = compactm_total_time / ITERATIONS;

      binu_av_time = binu_total_time / ITERATIONS;
      compactu_av_time = compactu_total_time / ITERATIONS;

      cout << "Average Marshall String len3: Binary-> " << binm_av_time << " Compact-> " << compactm_av_time << endl;

      cout << "Average Umarshall string len3: binary-> " << binu_av_time << " Compact -> " << compactu_av_time << endl;

      //std::cout << "Greeter received: " << reply << std::endl;
      break;
    }

    // round trip time for small message
    // measure request + response time
    // run multiples time to see if first request takes longer than subsequent requests
    case 5: {
      struct timespec start, stop;
      int32_t len1 = 512;
      int32_t len2 = 1024;
      int32_t len3 = 2048;
      double average_time = 0;
      double time_sum = 0;

      std::cout << " *** TESTING INT *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_TYPE, & start);
        std::int32_t intReq = 1;
        client.AcceptInt(reply_str, intReq);
        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Round trip time to send integer: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        time_sum += (stop.tv_nsec - start.tv_nsec);
      }
      average_time = time_sum / ITERATIONS;
      cout << "Average time for int: " << average_time << endl;
      average_time = 0;
      time_sum = 0;

      std::cout << " *** TESTING DOUBLE *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_TYPE, & start);
        std::double_t doubleReq = 1.0;
        client.AcceptDouble(reply_str, doubleReq);
        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Round trip time to send double : " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        time_sum += (stop.tv_nsec - start.tv_nsec);
      }
      average_time = time_sum / ITERATIONS;
      cout << "Average time for double: " << average_time << endl;
      average_time = 0;
      time_sum = 0;

      std::cout << " *** TESTING STRING OF LEN = 512 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_TYPE, & start);
        std::string stringReq = custom_helper.CreateString(len1);
        client.AcceptString(reply_str, stringReq);
        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Round trip time to send string message of 512: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        time_sum += (stop.tv_nsec - start.tv_nsec);
      }
      average_time = time_sum / ITERATIONS;
      cout << "Average time for str len 512: " << average_time << endl;
      average_time = 0;
      time_sum = 0;

      std::cout << " *** TESTING STRING OF LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_TYPE, & start);
        std::string stringReq = custom_helper.CreateString(len2);
        client.AcceptString(reply_str, stringReq);
        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Round trip time to send small message of 1024: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        time_sum += (stop.tv_nsec - start.tv_nsec);
      }
      average_time = time_sum / ITERATIONS;
      cout << "Average time for str len 1024: " << average_time << endl;
      average_time = 0;
      time_sum = 0;

      std::cout << " *** TESTING STRING OF LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_TYPE, & start);
        std::string stringReq = custom_helper.CreateString(len3);
        client.AcceptString(reply_str, stringReq);
        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Round trip time to send small message of 2048: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        time_sum += (stop.tv_nsec - start.tv_nsec);
      }
      average_time = time_sum / ITERATIONS;
      cout << "Average time for str len 2048: " << average_time << endl;
      average_time = 0;
      time_sum = 0;

      std::cout << " *** TESTING COMPLEX DATA STRUCTURE 1 *** " << std::endl;
      std::cout << " *** CONTENTS: INT, DOUBLE, STRING LEN = 512 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_TYPE, & start);
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = custom_helper.CreateString(len1);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        client.AcceptComplexDataStructure(reply_str, complex_ds);
        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Round trip time to send complex 1 message: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        time_sum += (stop.tv_nsec - start.tv_nsec);
      }
      average_time = time_sum / ITERATIONS;
      cout << "Average time for complex 1: " << average_time << endl;
      average_time = 0;
      time_sum = 0;

      std::cout << " *** TESTING COMPLEX DATA STRUCTURE 2 *** " << std::endl;
      std::cout << " *** CONTENTS: INT, DOUBLE, STRING LEN = 1024 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_TYPE, & start);
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = custom_helper.CreateString(len2);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        client.AcceptComplexDataStructure(reply_str, complex_ds);
        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Round trip time to send complex 2 message: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        time_sum += (stop.tv_nsec - start.tv_nsec);
      }
      average_time = time_sum / ITERATIONS;
      cout << "Average time for complex 2: " << average_time << endl;
      average_time = 0;
      time_sum = 0;

      std::cout << " *** TESTING COMPLEX DATA STRUCTURE 3 *** " << std::endl;
      std::cout << " *** CONTENTS: INT, DOUBLE, STRING LEN = 2048 *** " << std::endl;
      for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_TYPE, & start);
        std::int32_t intReq = 1;
        std::double_t doubleReq = 1.0;
        std::string stringReq = custom_helper.CreateString(len3);
        ComplexDataStructure complex_ds;
        complex_ds.val1 = intReq;
        complex_ds.val2 = doubleReq;
        complex_ds.val3 = stringReq;
        client.AcceptComplexDataStructure(reply_str, complex_ds);
        clock_gettime(CLOCK_TYPE, & stop);
        std::cout << "Round trip time to send complex 3 message: " << (stop.tv_sec - start.tv_sec) << " s " <<
          (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
        time_sum += (stop.tv_nsec - start.tv_nsec);
      }
      average_time = time_sum / ITERATIONS;
      cout << "Average time for complex 3: " << average_time << endl;
      average_time = 0;
      time_sum = 0;

      break;
    }

    // large messages without streaming
    case 6: {
      struct timespec start, stop;
      int sizeArr[] = {
        128,
        256,
        512,
        1024,
        2048,
        4096,
        8192,
        16384,
        32768,
        65536,
        131072
      };
      int len = sizeof(sizeArr) / sizeof(sizeArr[0]);
      double average_time = 0;
      double time_sum = 0;

      int test_iterations = 5;

      for (int i = 0; i < len; i++) {
        std::cout << " *** TESTING STRING OF LEN = " << sizeArr[i] << " *** " << std::endl;
        for (int j = 0; j < test_iterations; j++) {
          clock_gettime(CLOCK_TYPE, & start);
          std::string req = custom_helper.CreateString(sizeArr[i]);
          client.AcceptString(reply_str, req);
          clock_gettime(CLOCK_TYPE, & stop);
          std::cout << "Round trip time to send large message : " << (stop.tv_sec - start.tv_sec) << " s " <<
            (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
          time_sum += (stop.tv_nsec - start.tv_nsec);
        }
        average_time = time_sum / test_iterations;
        cout << "Average time for len:" << sizeArr[i] << ": " << average_time << endl;
        average_time = 0;
        time_sum = 0;
      }

      break;
    }

    // large message w client streaming
    case 7: {
      struct timespec start, stop;
      int sizeArr[] = {
        128,
        256,
        512,
        1024,
        2048,
        4096,
        8192,
        16384,
        32768,
        65536,
        131072
      };
      int len = sizeof(sizeArr) / sizeof(sizeArr[0]);
      double average_time = 0;
      double time_sum = 0;
      for (int i = 0; i < len; i++) {
        std::cout << " *** TESTING STRING OF LEN = " << sizeArr[i] << " *** " << std::endl;
        for (int j = 0; j < ITERATIONS; j++) {
          clock_gettime(CLOCK_TYPE, & start);

          ////////////////////////////////////////////////////

          int streamIterations = 8;

          std::string toSend = custom_helper.CreateString(sizeArr[i] / streamIterations);

          for (int i = 0; i < streamIterations; i++) {
            client.AcceptClientSideStream(reply_str, toSend);
          }
          ////////////////////
          clock_gettime(CLOCK_TYPE, & stop);
          std::cout << "Round trip time to send large message with client straming : " << (stop.tv_sec - start.tv_sec) << " s " <<
            (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
          time_sum += (stop.tv_nsec - start.tv_nsec);
        }

        average_time = time_sum / ITERATIONS;
        cout << "Average time for len for client streaming:" << sizeArr[i] << ": " << average_time << endl;
        average_time = 0;
        time_sum = 0;

      }

      break;
    }

    case 8: {
      struct timespec start, stop;
      int sizeArr[] = {
        1000000,
        10485760,
        20971520,
        31475280
      };
      int len = sizeof(sizeArr) / sizeof(sizeArr[0]);
      double average_time = 0;
      double time_sum = 0;

      int test_iterations = 3;

      for (int i = 0; i < len; i++) {
        std::cout << " *** TESTING STRING OF LEN = " << sizeArr[i] << " *** " << std::endl;
        for (int j = 0; j < test_iterations; j++) {
          clock_gettime(CLOCK_TYPE, & start);
                  ////////////////////////////////////////////////////

          int streamIterations = 8;

          std::string toSend = custom_helper.CreateString(sizeArr[i] / streamIterations);

          for (int i = 0; i < streamIterations; i++) {
            client.AcceptClientSideStream(reply_str, toSend);
          }
          ////////////////////
          clock_gettime(CLOCK_TYPE, & stop);
          std::cout << "Round trip time to send large message : " << (stop.tv_sec - start.tv_sec) << " s " <<
            (stop.tv_nsec - start.tv_nsec) << " ns" << std::endl;
          time_sum += (stop.tv_nsec - start.tv_nsec);
        }
        average_time = time_sum / test_iterations;
        cout << "Average time for len:" << sizeArr[i] << ": " << average_time << endl;
        average_time = 0;
        time_sum = 0;
      }

      break;
    }

    default: {
      std::cout << "Invalid argument for -t flag. Run -h to learn more" << std::endl;
      break;
    }
    }

  } catch (...) {
    std::cout << "Client caught an exception" << std::endl;
  }

  trans -> close();
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