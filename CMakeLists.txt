cmake_minimum_required(VERSION 3.5)
project(ferryqueue)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp ferry.h ferry.c dev/ferryMsg.h dev/ferryMsg.c thread/ferryThread.h thread/ferryThread.c)
add_executable(ferryqueue ${SOURCE_FILES})