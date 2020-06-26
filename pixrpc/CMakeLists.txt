cmake_minimum_required(VERSION 3.15)
project(pixrpc)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DLIB REQUIRED IMPORTED_TARGET dlib-1)

set(PIXRPC_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${PIXRPC_INCLUDE}")

add_library(pixrpc STATIC pixrpc.cpp)

target_link_libraries(pixrpc)
set(PIXRPC_INCLUDE ${PIXRPC_INCLUDE} PARENT_SCOPE)
set(PIXRPC_LIBRARY pixrpc ${DLIB_LIBRARIES} PARENT_SCOPE)

