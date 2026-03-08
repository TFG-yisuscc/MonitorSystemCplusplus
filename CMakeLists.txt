cmake_minimum_required(VERSION 4.1)
project(MonitorSystem)

set(CMAKE_CXX_STANDARD 20)
find_package(llama CONFIG REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
include_directories(includes)
add_executable(MonitorSystem src/main.cpp
        includes/metrics/promptmetrics.cpp
        includes/metrics/promptmetrics.h
        includes/utils/InferenceEngines.h
        includes/utils/promptParser.h
        src/clients/OllamaTest.cpp
        includes/clients/OllamaTest.h
        includes/metrics/promptmetriclogger.cpp
        includes/metrics/promptmetriclogger.h
        includes/clients/LlamaTest.cpp
        includes/clients/LlamaTest.h
        includes/metrics/hardwareMetrics.cpp
        includes/metrics/hardwareMetrics.h
        includes/metrics/promptmetricsLlama.h
        includes/utils/ProbabilityType.h)
target_link_libraries(MonitorSystem PRIVATE llama nlohmann_json::nlohmann_json)
