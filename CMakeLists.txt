cmake_minimum_required(VERSION 3.10)
project(ST_1)
#SET(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g")

add_executable(ST_1 main.cpp RadialStitcher.cpp RadialStitcher.h)

find_package(OpenCV REQUIRED)
find_package(Threads)


target_link_libraries(ST_1 ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})