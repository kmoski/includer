cmake_minimum_required(VERSION 3.17)
project(includer)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE
    src/includer.cpp
    src/util/settings_parser.cpp
    src/output.cpp
    src/util/c_include_parser.cpp
    src/util/fs_hash.cpp
    src/settings.cpp)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE src/main.cpp
    PUBLIC  ${SOURCE})

add_subdirectory(test)