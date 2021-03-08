cmake_minimum_required(VERSION 3.17)
project(includer)

set(CMAKE_CXX_STANDARD 20)

add_executable(includer src/main.cpp src/includer.cpp src/includer.h)