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
./afs-client  -f <mount point>
```

(Note: -f let's you print debug statements while running fuse). Do Ctrl + C to stop ./afs-client.


## To set the file system:
Note: Do it from the terminal ONLY (do not use VSCode or any editor)
```
mkdir -p /tmp/afs
cd <mount point>
<any terminal command>
```

## General
Where are the local files stored?
/tmp/afs

## Misc
Inorder to run cmake, change the location of the downloaded FUSE path
in common/FindFUSE.cmake
