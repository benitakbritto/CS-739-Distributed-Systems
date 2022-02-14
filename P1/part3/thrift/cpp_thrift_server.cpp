#include "gen-cpp/CustomSvc.h"
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <memory>
#include <iostream>

using namespace ::apache::thrift::server;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace std;

using std::shared_ptr;
using std::make_shared;

class CustomSvcHandler : public CustomSvcIf {
public:
  CustomSvcHandler() {
    // Your initialization goes here
    // Add ping details here :
    cout << "initialization here";
  }


    virtual void getHelloMessage(std::string& _return, const std::string& name) override {
        cout << "Server received: " << name << ", from client" << endl;
        _return = "Hello " + name;
    }

    virtual void AcceptInt(std::string& _return, const int32_t num){
        cout<< "Received int: " << num << endl;
        _return = "Received Integer";
        // return "Received integer";
    }

    virtual void AcceptDouble(std::string& _return, const double num){
        cout<< "Received double: " << num << endl;
        _return = "Received double";
    }

    virtual void AcceptString(std::string& _return, const string& string_message){
        // cout<< "Received string: " << string_message << endl;
        //cout<< "Received string" << endl;
        _return = "Received string";
    }

    virtual void AcceptComplexDataStructure(std::string& _return, const ComplexDataStructure& complex_data){
        cout << "Received Complex data structure " << endl;
        _return = "Received Complex Data structure";
    }

    virtual void AcceptClientSideStream(std::string& _return, const string& streaming_string){
        // cout << "Received streaming data: " << streaming_string << endl;
        cout << "Received streaming data:" << endl;
        _return = "Received client streaming data";
    }
    
};


int main() {
    auto handler = make_shared<CustomSvcHandler>();
    auto proc = make_shared<CustomSvcProcessor>(handler);
    auto trans_svr = make_shared<TServerSocket>(5050);
    auto trans_fac = make_shared<TBufferedTransportFactory>();
    auto proto_fac = make_shared<TBinaryProtocolFactory>();
    TSimpleServer server(proc, trans_svr, trans_fac, proto_fac);
    server.serve();
    return 0;
}