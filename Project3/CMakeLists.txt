cmake_minimum_required(VERSION 3.17)
project(Project3)

set(CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS "-pthread")

add_executable(Project3 main.cpp Human.cpp Human.h Graph.h Graph.cpp)