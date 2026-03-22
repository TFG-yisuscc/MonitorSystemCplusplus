cmake_minimum_required(VERSION 3.25.1)
project(MonitorSystem)

set(CMAKE_CXX_STANDARD 20)
find_package(llama CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
include_directories(includes)
add_executable(MonitorSystem src/main.cpp
        includes/metrics/promptmetrics.cpp
        includes/metrics/promptmetrics.h
        includes/utils/InferenceEngines.h
        includes/utils/promptParser.h
        src/clients/OllamaTest.cpp
        includes/clients/OllamaTest.h
        includes/metrics/Logger.cpp
        includes/metrics/Logger.h
        src/clients/LlamaTest.cpp
        includes/clients/LlamaTest.h
        includes/metrics/hardwareMetrics.cpp
        includes/metrics/hardwareMetrics.h
        includes/metrics/promptmetricsLlama.h
        includes/utils/ProbabilityType.h
        includes/clients/LlamaServerTest.cpp
        includes/clients/LlamaServerTest.h
        src/utils/LlamaInferencer.cpp
        includes/utils/LlamaInferencer.h
        src/main.cpp
        includes/utils/cpuMonitor.h
        includes/utils/cpuTicks.h
        includes/utils/misc.h
)
target_link_libraries(MonitorSystem PRIVATE llama nlohmann_json::nlohmann_json fmt::fmt)
