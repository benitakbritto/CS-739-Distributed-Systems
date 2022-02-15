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
<num> lies in the [0,3]
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
mv ../snappy_test_tool.cc .
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

# part 3 - grpc:
## SETUP
```
$ export MY_INSTALL_DIR=$HOME/.local
$ mkdir -p $MY_INSTALL_DIR
$ export PATH="$MY_INSTALL_DIR/bin:$PATH"
```

## BUILD FOR GRPC AND PROTOBUF
```
$ cd part3/grpc
$ mkdir -p cmake/build
$ pushd cmake/build
$ cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
      ../..
$ make -j 4
$ make install
$ popd
```

## BUILD THE PROJECT CODE
```
$ cd examples/cpp/helloworld
$ mkdir -p cmake/build
$ pushd cmake/build
$ cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..
$ make -j 4
```

## RUN THE SERVER
```
$ ./greeter_server
```

TODO: Add description of command line args
## RUN THE CLIENT (ON ANOTHER TERMINAL)
```
$ ./greeter_client <command line args>
```

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
