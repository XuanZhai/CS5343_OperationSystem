cmake_minimum_required(VERSION 3.17)
project(Project2 C)

set (CMAKE_C_FLAGS "-pthread")
set (input02 "input.txt")

set(CMAKE_C_STANDARD 99)

add_executable(Project2 Zhai_James_prog2.c)