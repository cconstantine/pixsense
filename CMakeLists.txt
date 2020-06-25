
cmake_minimum_required(VERSION 3.0)
project(pixsense)

find_package(realsense2 REQUIRED )

find_package(OpenPose REQUIRED)
find_package( Threads REQUIRED )
find_package( GFlags REQUIRED )
include_directories(${OpenPose_INCLUDE_DIRS})

add_subdirectory("pixrpc/")
include_directories("${PIXRPC_INCLUDE}")

include_directories("include/")

add_executable(tracking src/tracking.cpp src/tracking_service.cpp src/eye_tracker.cpp src/face_finder.cpp)
target_link_libraries(tracking realsense2 ${GFLAGS_LIBRARIES} ${OpenPose_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${PIXRPC_LIBRARY})
