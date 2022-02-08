# part 0
## Compile
```
gcc -o part0 part0.c -Wall
```

TODO: Add description of command line args
## Run
```
./part0 <command line arg>
```

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

