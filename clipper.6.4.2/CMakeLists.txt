
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

include_directories(${CMAKE_SRC_DIR}/src)
add_library(clipper src/clipper.cpp)

add_executable(test src/test.cpp)
target_link_libraries(test clipper)
