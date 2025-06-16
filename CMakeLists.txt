cmake_minimum_required(VERSION 3.10)
project(MetricLogger)

option(BUILD_TEST_1 "Build test1 (test1.cpp)" ON)
option(BUILD_TEST_2 "Build test2 (test2.cpp)" OFF)

set(CXX_STANDARD 17)

# set(SOURCES
#     src/metric_manager.cpp
#     src/cnt_metric.cpp
#     src/avg_metric.cpp
#     src/cpu_utilization_metric.cpp
# )

add_library(metric_logger_lib
    src/metric_manager.cpp
    src/cnt_metric.cpp
    src/avg_metric.cpp
    src/cpu_utilization_metric.cpp
)

target_include_directories(metric_logger_lib PUBLIC src)
find_package(Threads REQUIRED)
target_link_libraries(metric_logger_lib PRIVATE Threads::Threads)

if(BUILD_TEST_1)
    message(STATUS "Building test target: Test1")
    add_executable(test1
        src/test1.cpp
    )
    target_link_libraries(test1 PRIVATE metric_logger_lib)
endif()

if(BUILD_TEST_2)
    message(STATUS "Building test target: Test2")
    add_executable(test2
        src/test2.cpp
    )
    target_link_libraries(test2 PRIVATE metric_logger_lib)
endif()

