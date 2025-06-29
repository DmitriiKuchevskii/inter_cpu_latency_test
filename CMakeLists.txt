cmake_minimum_required(VERSION 3.31)
project(inter_cpu_latency_test)

set(CMAKE_CXX_STANDARD 20)

add_executable(inter_cpu_latency_test)
target_sources(inter_cpu_latency_test
        PUBLIC
        main.cpp
)
target_sources(inter_cpu_latency_test
        PUBLIC
        FILE_SET CXX_MODULES FILES
        SPSCRingBuffer.ixx
        LatenciesWriter.ixx
        HwTopology.ixx
        CpuBondedThread.ixx
        InterCpuLatencyTest.ixx
)

target_link_libraries(inter_cpu_latency_test hwloc)