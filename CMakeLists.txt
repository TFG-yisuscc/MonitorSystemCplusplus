cmake_minimum_required(VERSION 4.1)
project(MonitorSystem)

set(CMAKE_CXX_STANDARD 20)
find_package(llama CONFIG REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
include_directories(includes)
add_executable(MonitorSystem src/main.cpp
        includes/metrics/promptmetrics.cpp
        includes/metrics/promptmetrics.h
        src/utils/prompt_utils.cpp
        includes/utils/prompt_utils.h
        includes/utils/InferenceEngines.h
        src/utils/promptParser.h)
target_link_libraries(MonitorSystem PRIVATE llama nlohmann_json::nlohmann_json)
