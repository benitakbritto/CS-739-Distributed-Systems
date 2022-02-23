# Minimum CMake required
cmake_minimum_required(VERSION 3.15)

# C++ version
set(CMAKE_CXX_STANDARD 17)

# Project
project(filesystemcomm)

# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# gRPC
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file
get_filename_component(hw_proto "filesystemcomm.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/filesystemcomm.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/filesystemcomm.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/filesystemcomm.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/filesystemcomm.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Targets (client|server)
foreach(_target
  client server)
  add_executable(${_target} "${_target}.cc"
    ${hw_proto_srcs}
    ${hw_grpc_srcs})
  target_link_libraries(${_target}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()
