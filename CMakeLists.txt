cmake_minimum_required(VERSION 3.25.1)
project(MonitorSystem)

set(CMAKE_CXX_STANDARD 20)
#uso el json que viene edefinido en el header de ollama, (problemas decompatibilidad de abi1)
find_package(llama CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
include_directories(includes)
add_executable(MonitorSystem src/main.cpp
        src/metrics/promptmetrics.cpp
        includes/metrics/promptmetrics.h
        includes/utils/InferenceEngines.h
        includes/utils/promptParser.h
        src/clients/OllamaTest.cpp
        includes/clients/OllamaTest.h
        src/metrics/Logger.cpp
        includes/metrics/Logger.h
        src/clients/LlamaTest.cpp
        includes/clients/LlamaTest.h
        src/metrics/hardwareMetrics.cpp
        includes/metrics/hardwareMetrics.h
        includes/utils/ProbabilityType.h
        src/clients/LlamaServerTest.cpp
        includes/clients/LlamaServerTest.h
        src/utils/LlamaInferencer.cpp
        includes/utils/LlamaInferencer.h
        includes/utils/cpuMonitor.h
        includes/utils/cpuTicks.h
        includes/utils/misc.h
        src/utils/inputConfiguration.cpp
        includes/utils/inputConfiguration.h
        includes/utils/enumConfig.h
        includes/utils/enumConfig.h
        src/utils/hardwareMeasurements.cpp
        includes/utils/hardwareMeasurements.h
)
target_link_libraries(MonitorSystem PRIVATE llama fmt::fmt)
target_link_options(MonitorSystem PRIVATE -static) #enlazado estatico

configure_file(${CMAKE_SOURCE_DIR}/config_example.json
               ${CMAKE_BINARY_DIR}/config_example.json COPYONLY)
