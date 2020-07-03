
cmake_minimum_required(VERSION 3.0)
project(pixsense)

option(BUILD_TESTS "Build tests program" OFF)

find_package(realsense2 REQUIRED )

find_package(OpenPose REQUIRED)
find_package( Threads REQUIRED )
find_package( GFlags REQUIRED )
include_directories(${OpenPose_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(DLIB REQUIRED IMPORTED_TARGET dlib-1)

add_subdirectory("pixrpc/")
include_directories("${PIXRPC_INCLUDE}")

include_directories("include/")

add_library(pixsense src/eye_tracker.cpp src/face_finder.cpp src/person.cpp src/tracked_face.cpp)
link_libraries(pixsense realsense2 ${OpenPose_LIBS})

add_executable(tracking src/tracking.cpp )
target_link_libraries(tracking
  ${GFLAGS_LIBRARIES}
  ${PIXRPC_LIBRARY}
)

add_executable(client src/client.cpp)
target_link_libraries(client ${PIXRPC_LIBRARY})

if (BUILD_TESTS)
  find_package(Catch2 REQUIRED)
  add_executable(tests tests/simple.cpp)
  target_link_libraries(tests Catch2::Catch2 pixsense )
endif()