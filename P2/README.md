# Installing grpc 
export MY_INSTALL_DIR=$HOME/.local
mkdir -p $MY_INSTALL_DIR
export PATH="$MY_INSTALL_DIR/bin:$PATH"

sudo apt install -y cmake
cmake --version
sudo apt install -y build-essential autoconf libtool pkg-config

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

# Building src (first time only)
cd src
mkdir -p cmake/build
cd cmake/build
cmake ../.. 
make

# Building src 
Note: pwd is src/cmake/build
make

# Run 
Server: 
./server

Client:
./client