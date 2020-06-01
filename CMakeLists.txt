
cmake_minimum_required(VERSION 3.0)
project(pixsense)

find_package(realsense2 REQUIRED )

find_package(OpenPose REQUIRED)
include_directories(${OpenPose_INCLUDE_DIRS})

find_package( Threads REQUIRED )

find_package( GFlags REQUIRED )


add_executable(tracking tracking.cpp)
target_link_libraries(tracking realsense2 ${GFLAGS_LIBRARIES} ${OpenPose_LIBS} ${CMAKE_THREAD_LIBS_INIT})
