
cmake_minimum_required(VERSION 3.0)
project(pixsense)

option(BUILD_TESTS "Build tests program" OFF)

find_package(realsense2 REQUIRED )

find_package(OpenPose REQUIRED)
find_package( Threads REQUIRED )
find_package( GFlags REQUIRED )
find_package(PostgreSQL REQUIRED )

include_directories(${OpenPose_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(DLIB REQUIRED IMPORTED_TARGET dlib-1)

include_directories("include/")

add_library(pixsense
  src/eye_tracker.cpp
  src/face_finder.cpp
  src/mob.cpp
  src/person.cpp
  src/tracked_face.cpp
)
link_libraries(pixsense realsense2 ${OpenPose_LIBS})

add_subdirectory("pixpq/")
include_directories("${PIXPQ_INCLUDE}")

add_executable(tracking src/tracking.cpp )
target_link_libraries(tracking
  ${GFLAGS_LIBRARIES}
  ${DLIB_LIBRARIES}
  ${PIXPQ_LIBRARY}
)

if (BUILD_TESTS)
  find_package(Catch2 REQUIRED)
  add_executable(tests tests/tests.cpp
    tests/mob_tests.cpp
    tests/person_tests.cpp
    tests/simple.cpp
  )
  target_link_libraries(tests Catch2::Catch2 pixsense )
endif()