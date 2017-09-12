cmake_minimum_required(VERSION 3.9)

# clipper
include_directories(clipper.6.4.2/src)
add_subdirectory(clipper.6.4.2/)

add_executable(clipper_benchmark benchmark/clipper.cpp)
target_link_libraries(clipper_benchmark clipper)

# gpc
include_directories(gpc.2.33 benchmark)
add_subdirectory(gpc.2.33)

add_executable(gpc_benchmark benchmark/gpc.c)
target_link_libraries(gpc_benchmark gpc)

# generator
add_executable(generator benchmark/generator.c)
target_link_libraries(generator m)

#shp2gpf
add_executable(shp2gpf utils/shp2gpf.c)
target_link_libraries(shp2gpf shp proj)
