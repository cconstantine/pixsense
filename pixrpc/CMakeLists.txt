cmake_minimum_required(VERSION 3.15)
project(pixrpc)

###################### GRPC ##################################
# add_subdirectory(./ext/grpc ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

set(_PROTOBUF_PROTOC protoc)
set(_GRPC_CPP_PLUGIN_EXECUTABLE /usr/bin/grpc_cpp_plugin)


# Generated sources
set(proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/pixrpc.pb.cc")
set(proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/pixrpc.pb.h")
set(grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/pixrpc.grpc.pb.cc")
set(grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/pixrpc.grpc.pb.h")

set(PIXRPC_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}")

set(proto "${CMAKE_CURRENT_SOURCE_DIR}/pixrpc.proto")

add_custom_command(
      OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
      COMMAND "${_PROTOBUF_PROTOC}"
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}/"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${CMAKE_CURRENT_SOURCE_DIR}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${proto}"
      DEPENDS "${proto}")


include_directories("${PIXRPC_INCLUDE}")
include_directories(${PROTOBUF_INCLUDE_DIR})

add_library(pixrpc STATIC
  ${proto_srcs}
  ${grpc_srcs}
)

target_link_libraries(pixrpc gRPC::grpc++ protobuf )
set(PIXRPC_INCLUDE ${PIXRPC_INCLUDE} PARENT_SCOPE)
set(PIXRPC_LIBRARY pixrpc PARENT_SCOPE)

