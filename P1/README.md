# part 0

* Code Location: part0/part0.c
* Results Location: results/part0/{0-3}.txt

### Which clock to we use? 
We use clock_gettime() instead of gettimeofday() because it gives us precision in nanoseconds. This is beneficial as we need to conduct experiments that require nanosecond precision.

### Which clock type do we use for our experiments?
We use CLOCK_MONOTONIC because it is used for elapsed time calculation (a perfect use case for our project) and it is not affected by changes in system time-of-day clock.

### Compile
```
cd part0
gcc -o part0 part0.c -Wall
```

### Run
```
./part0 <num>
```
<num> lies in the [0,3]
* ./part0 0: Uses CLOCK_REALTIME to measure the execution time of simpleLoop()
* ./part0 1: Uses CLOCK_MONOTONIC to measure the execution time of simpleLoop()
* ./part0 2: Uses CLOCK_PROCESS_CPUTIME_ID to measure the execution time of simpleLoop()
* ./part0 3: Uses CLOCK_THREAD_CPUTIME_ID to measure the execution time of simpleLoop()
      

# part 1
```
gcc -o part1 part1.c -Wall
```

TODO: Add description of command line args
## Run
```
./part1 <command line arg>
```

# part 2
## Run
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

