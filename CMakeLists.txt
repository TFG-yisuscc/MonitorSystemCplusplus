cmake_minimum_required(VERSION 4.1)
project(MonitorSystem)

set(CMAKE_CXX_STANDARD 20)
find_package(llama CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
include_directories(includes)
add_executable(MonitorSystem src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)