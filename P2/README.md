# Installing gRPC
```
export MY_INSTALL_DIR=$HOME/.local
mkdir -p $MY_INSTALL_DIR
export PATH="$MY_INSTALL_DIR/bin:$PATH"
```

```
sudo apt install -y cmake
cmake --version
sudo apt install -y build-essential autoconf libtool pkg-config
```

```
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
```

# Install FUSE
Follow: https://github.com/libfuse/libfuse#installation


# Building the project 
```
cd src
mkdir -p build
cd build
cmake .. 
make
```

# Run the project 

## Server: 
```
./afs-server <path of server root dir>
```

Alternative
Run server in the background 
```
./server <path of server root dir> &
```

Make sure to kill the process if you are running the server in the background
```
kill -9 <pid>
```

To get the pid use the following command
```
ps -e | grep "afs-server"
```

## Client:
```
./afs-client  -f <mount point> <client cache>
```

(Note: -f let's you print debug statements while running fuse). Do Ctrl + C to stop ./afs-client.


## To test the file system:
Note: Do it from the **terminal ONLY** (do not use VSCode or any editor's terminal or an editor's build functionality. Why? The editor's FS calls will be intercepted by FUSE.)
```
mkdir -p /tmp/afs
cd <mount point>
<any terminal command>
```

## Misc
To run cmake, change the location of the downloaded FUSE path in common/FindFUSE.cmake

## Rubric
1.1. Posix Compliance
- Location: src/afs_fuse.cc

1.2. AFS Protocol and Semantics
- Location (client - grpc): src/afs_client.h
- Functions: MakeDir(), CreateFile(), RemoveDir(), ReadDir(), GetFileStat(), DeleteFile(), OpenFile(), CloseFile(), TestAuth(), CloseFileWithStream(), OpenFileWithStream()

- Location (server - grpc): src/server.cc
- Functions: Fetch(), Store(), GetFileStat(), TestAuth(), MakeDir(), Mknod(), RemoveDir(), ReadDir(), StoreWithStream(), FetchWithStream()

- Location (protobuf - grpc): src/filesystemcomm.proto

1.3. Durability and 1.4 Crash Recovery Protocol
- Location (client): src/afs_fuse.cc
- Functions: init_single_log(), write_single_log(), createPendingFile(), init_multi_log()

- Location (client): src/fs_client.h
- Functions: checkModified_single_log(), isFileModifiedv2(), closeEntry_single_log(), removePendingFile()

- Location (server): src/server.cc
- Functions: write_file()

- We use CRASH_TEST in src/afs_client.h and src/server cc to add simulated crash points around our update protocols. 

2.1 Performance
- Location: src/server.cc
- Functions: put_file_in_mem_map(), get_file_from_mem_map(), delete_file_from_mem_map(), partial_free_mem_map(), StoreWithStream(), FetchWithStream()

- Location: src/afs_client.h
- Functions: OpenFileWithStream(), CloseFileWithStream()




