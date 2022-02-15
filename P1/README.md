# part 0

* Code Location: part0/part0.c
* Results Location: results/part0/{0-3}.txt

### Which clock to we use? 
We use clock_gettime() instead of gettimeofday() because it gives us precision in nanoseconds. This is beneficial as we need to conduct experiments that require nanosecond precision.

### Which clock type do we use for our experiments?
We use CLOCK_MONOTONIC because it is used for elapsed time calculation (a perfect use case for our project) and it is not affected by changes in system time-of-day clock.

### Compile
Note: pwd is P1/part0
```
gcc -o part0 part0.c -Wall
```

### Run
Note: pwd is P1/part0
```
./part0 <num>
```
num lies in the [0,3]
* ./part0 0: Uses CLOCK_REALTIME to measure the execution time of simpleLoop()
* ./part0 1: Uses CLOCK_MONOTONIC to measure the execution time of simpleLoop()
* ./part0 2: Uses CLOCK_PROCESS_CPUTIME_ID to measure the execution time of simpleLoop()
* ./part0 3: Uses CLOCK_THREAD_CPUTIME_ID to measure the execution time of simpleLoop()
      

# part 1
### L1 cache reference
* Code Location: part1/part1.c
* Measurement: 0.515 ns

### Branch mispredict
* Code Location: part1/part1.c
* Measurement: 9 ns

### L2 cache reference
* Code Location: part1/part1.c
* Measurement: 0.58 ns

### Mutex lock/unlock
* Code Location: part1/part1.c
* Measurement: 
	* Lock: 36.36 ns, Unlock: 18.34 ns (On considering all 100 iter)
	* Lock: 18.494949494949495 ns, Unlock: 18.0 ns (On ignoring first run)

### Main memory reference
* Code Location: part1/part1.c
* Measurement: 
	* Avg: 275.28 ns
      * Median: 163.5 ns

### Compress 1K bytes with Zippy
* Code Location: snappy_test_tool.cc
* Measurement: 
	* File with a's: 537,424.09 ns
	* Random file: 629,614.43 ns

### Send 1K bytes over 1 Gbps network
* Code Location: part2/client.c
* Measurement: 40,000 ns

### Read 4K randomly from SSD*
* Code Location: part1/part1.c
* Measurement: 7,588.65 ns

### Read 1 MB sequentially from memory
* Code Location: part1/part1.c
* Measurement: 357,071.7049 ns

### Round trip within same datacenter
* Command: ping ${ip_address_of_machine_in_same_datacentre}
* Measurement: 180,000 ns

### Read 1 MB sequentially from SSD*
* Code Location: part1/part1.c
* Measurement: 803,255.65 ns

### Disk seek
* Code Location: part1/part1.c
* Measurement: 
	* Avg: 833.0 ns
      * Median: 600.0 ns

### Read 1 MB sequentially from disk
* Code Location: part1/part1.c
* Measurement: 
	* Avg: 93,334.36 ns
      * Median: 79,950.0 ns

###  Send packet CA->Netherlands->CA
* Command: ping ${public_ip_address_of_netherland:wq}
* Measurement: 114,000,000 ns

### Compile
* For part1.c:
Note: pwd is P1/part1
```
gcc -o part1 part1.c -Wall
```

* For snappy_test_tool.cc
Note: pwd is P1/part1
```
git clone https://github.com/google/snappy.git
cd snappy
mv ../snappy_test_tool.cc snappy_test_tool.cc
cd build
make
```

### Run
* For part1.c:
Note: pwd is P1/part1
```
./part1 <num>
```
where num lies in [1-14]
* ./part1 1: Runs L1 cache reference
* ./part1 2: Runs Branch mispredict
* ./part1 3: Runs L2 cache reference
* ./part1 4: Runs Mutex lock/unlock
* ./part1 5: Runs Main memory reference
* ./part1 6: Runs Compress 1K bytes with Zippy
* ./part1 7: Runs Send 1K bytes over 1 Gbps network
* ./part1 8: Runs Read 4K randomly from SSD
* ./part1 9: Runs Read 1 MB sequentially from memory
* ./part1 10: Runs Round trip within same datacenter
* ./part1 11: Runs Read 1 MB sequentially from SSD
* ./part1 12: Runs Disk seek
* ./part1 13: Runs Read 1 MB sequentially from disk
* ./part1 14: Runs Send packet CA->Netherlands->CA

* For snappy_test_tool.cc
Note: pwd is P1/part1/snappy/build
```
snappy_test_tool <path of file to be compressed>
```

# part 2
### Run
...
make
./serverbasic ${port_num} ${drop_%}
./client ${client_port} ${server_hostname} ${server_port}
...

Reliable Communication
Reason about the following,

A. Performance:

* Overhead of sending a message.
    File - client.c (line 50-54) time taken by sendto() call 

* Total round trip time of sending a message and receiving an ack
    File - client.c (line 50-61) time taken before sending mssg and after receiving ack
    Calculated by method timespec_diff (line 17-25)

* Bandwidth achieved when sending a large number of max-sized packets between sender and receiver.
    File - client.c (line 176) avg rtt is calculated in (line 164-170) 

* Above three running on a single machine vs two.
    We have all readings in 1 machine vs 2.
    
### Analysis
* We measure overhead by taking timing for sento() call. This method sends data on socket and returns. Non- blocking call.
* Overhead increases with increase in packet size as a fraction of rtt too because large packets require more time in buffer management, message copies, checksumming, as well as for controlling the network interface.
* On a single machine, one way processing overhead is ~20%-30% of round trip time. Similar overhead would be seen at server side. This overhead causes high end-to-end latencies and thus limits our bandwidth. For 2 machines, the network latency increases, hence overheads are not a very significant factor. They contribute by ~5-8% to rtt. For small messages, reducing the overhead would help us achieve more bandwidth.
* We observe a dip close in overhead around 1500-2k packet size (MTU)

### Explain what limits your bandwidth and how this can be done better.
* The communication bandwidth of a system is often measured by sending a virtually infinite stream from one node to another. We are sending a packet and waiting for acknowledgement from server.

* For VMs located at California and Iowa, we observed packets> 1500 bytes timing out even when server drop % was set to 0. The network congestion and distance between the machines also impacts the throughput since UDP is connectionless.
* Our bandwidth is limited by packet _size and non streaming fashion.
* We can improve bandwidth by sending packets in streaming fashion and by reducing processing overhead.


B. Reliability:

* Demonstrate your library works by controlled message drops.
    Timeout set by setsockopt() and checked by err code (line 66) 
    Retries are implemented in client.c (line 146-153)

* The receiver code must be equipped to drop a percentage of packets randomly.
    Message drops in serverbasic.c (line 22-24)

* Measure round-trip time for various drop percentages while sending a stream of packets.
    RTT calculation in client.c (line 154, 165, 169)

C. Compiler Optimization:
For the above experiments, what’s the observation you made for compiler optimization’s influence on performance? 
    Changes made to Makefile compilation command with optimization flags O1, O2, O3, Os, Ofast
    We experimented with optimizations -O1, -O2, -O3, -Os and -Ofast. The size of the binary decreased and there was a slight improvement in rtt and bandwidth measurements but not too significant.


# part 3
### General 
* For marshaling and unmarshaling we test on int, double, strings of lengths 512, 1024 and 2048, and complex data types 1 (int, double, string len = 512), 2 (int, double, string len = 1024), 3 (int, double, string len = 2048)
* For small messages, we test on the same message types as above.
* We test on string of varying lengths (128 to 30 MB) for large messages.
* We approximately calculate Request and Response times as follows:
	* Request Time = Time to marshal request message + Network Time + Time to marshal request message
	* Response Time = Time to marshal response message + Network Time + Time to marshal response message
	* We obtain the (2 * Network Time) on getting the RTT of small messages and the Marshal & Unmarshal times from Part3-1 and Part3-2 sections as RTT = Request Time + Response Time.
* For streaming, we implement client side streaming as we need to experiment with sending large messages from the client. We implement 2 types of chunking mechanisms for streaming. Client Streaming 1 divides the string into 128 byte streams while Client Streaming 2 always divides a string into 8 parts. 
* We use optimization -O0 and -O3.
	
### Analysis
* We notice that Thrift performs better than gRPC for marshaling and unmarhaling.
* For complex data types, we observe that gRPC does better than the expected time while Thrift does not (Expected time for complex type = actual time taken to marshal int + actual to time taken to marshal double + actual time taken to marshal string. Actual time for complex type = Time measure by our experiment). 
* Unmarshaling takes longer than marshaling, for gRPC and Thrift. 
* Thrift performs better than gRPC for smaller messages (giving better RTT). Possible reason: gRPC’s multiplexing, application-level protocol.
* gRPC performs bettern than Thrift for larger messages (giving better bandwidth). Possible reason: gRPC’s multiplexing.
* gRPC bandwidth saturates at ~60KB without streaming. Client streaming for gRPC and Thrift seem to saturate around ~10 MB.
* UDP outforms gRPC and Thrift. Possible reason: Less overhead due to connection-less protocol. 
* Compiler optimization provides better results. 

# part 3 - gRPC
### Marshaling and unmarshaling measurements of int, double, string (of varying size) and a complex structure on both the platforms. * Code Location: P1/part3/grpc/helloworld.pb.cc
	* Marshaling: <Message Type>::_InternalSerialize()
	* Unmarshaling: <Message Type>::_InternalParse()

### Round trip time for a small message. Compare it against subsequent latencies. Divide the latency to request and response times. Do this on both single and two machines. Compare both the platforms against Part 2 A.
* Code Location: P1/part3/grpc/greeter_{client,server}.cc

### Total bandwidth achieved for large amounts of data on Thrift and grpc. Using server streaming and client streaming. How large do messages have to be to reach max bandwidth. How does this compare against Part 2 B.
* Code Location: P1/part3/grpc/greeter_{client,server}.cc


### Compare against compiler optimizations. Highlight differences if any.
* Code Location: CMakeLists.txt
* Code Change: set add_compile_options(-O3)


### Setup
pwd: P1/part3/grpc
```
export MY_INSTALL_DIR=$HOME/.local
mkdir -p $MY_INSTALL_DIR
export PATH="$MY_INSTALL_DIR/bin:$PATH"
git clone --recurse-submodules -b v1.43.0 https://github.com/grpc/grpc
cd grpc
mkdir -p cmake/build
pushd cmake/build
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
      ../..
make -j 4
make install
popd
mv ../helloworld.proto examples/protos/helloworld.proto
mv../greeter_client.cc examples/cpp/helloworld/greeter_client.cc
mv ../greeter_server.cc examples/cpp/helloworld/greeter_server.cc
cd examples/cpp/helloworld
mkdir -p cmake/build
pushd cmake/build
cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..
make -j 4
mv ../../../../../../helloworld.pb.cc helloworld.pb.cc
make -j 4
```

### Run
pwd: P1/grpc/grpc/examples/cpp/helloworld/cmake/build
* Run the server:
```
./greeter_server
```
	
* Run the client:
This shows the how to invoke the test cases
```
./greeter_client -h 
```

Invokes the test case
```
./greeter_client -t <num>
```
num lies in [1-8]
* ./greeter_client -t 1: Marshal & Unmarshall Int
* ./greeter_client -t 2: Marshal & Unmarshall Double
* ./greeter_client -t 3: Marshal & Unmarshall Strings
* ./greeter_client -t 4: Marshal & Unmarshall Complex Data Structures
* ./greeter_client -t 5: Round trip time of small messages
* ./greeter_client -t 6: Round trip time of large messages (without streaming)
* ./greeter_client -t 7: Round trip time of large messages (with client side streaming 1)
* ./greeter_client -t 8: Round trip time of large messages (with client side streaming 2)

# part 3 - thrift 

## SETUP
```
$ mkdir -p thrift
$ wget https://dlcdn.apache.org/thrift/0.15.0/thrift-0.15.0.tar.gz
$ tar -xvf thrift-0.15.0.tar.gz
$ sudo apt-get install automake bison flex g++ git libboost-all-dev libevent-dev libssl-dev libtool make pkg-config 
```

## BUILD FOR GRPC AND PROTOBUF
```
$ cd cd thrift-0.15.0
$ ./configure --with-cpp  --with-boost=/usr/local --with-python --without-csharp --without-java --without-erlang --without-perl --without-php --without-php_extension --without-ruby --without-haskell --without-go
$ make 
$ make install
```

## CREATE THE PROJECT CODE (thrift file)
```
$ mkdir mini_project
```
Create the thrift files and then run 
```
$ thrift --gen cpp mini_project.thrift
```
This will generate the stubs necessary for the cpp client and server

## RUN THE SERVER
```
$ ./cpp_server &
```

## RUN THE CLIENT

```
$ ./cpp_client_with_marshalling_opt -h
Usage: -t <num> to run test cases
where num is
1 = Testing Int
2 = Testing Double
3 = Testing Strings
4 = Testing Complex Data Structures
5 = Round trip time of small messages
6 = Round trip time of large messages (without streaming)
7 = Round trip time of large messages (with client side streaming)
8 = Varying size BW check (10MB, 20MB etc)

$ ./cpp_client_with_marshalling_opt -t <option_number>
```

# Miscellaneous 
### Machines Used
* CSL Machines
* Azure VM located in West US 2 (w HDD and w/o HDD)
* GCP VM located in Iowa
	
### Presentation
* Slide Deck: https://docs.google.com/presentation/d/1i-TvAvCnARsqQ6crZJyns8QpA7r1x_XLwjUb07Pxmxc/edit?usp=sharing
* Google Sheets (Results): https://docs.google.com/spreadsheets/d/1AiyX7XGcEiT_zEjRezv1x7HMzUYx2vdVW5pHhvUbOe4/edit?usp=sharing
