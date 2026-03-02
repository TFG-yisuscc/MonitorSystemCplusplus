cmake_minimum_required(VERSION 4.1)
project(MonitorSystem)

set(CMAKE_CXX_STANDARD 20)

include_directories(src/headers)

add_executable(MonitorSystem src/main.cpp)
