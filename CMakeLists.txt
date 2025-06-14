cmake_minimum_required(VERSION 3.10)
project(MetricLogger)

set(CXX_STANDARD 17)

set(SOURCES
    src/main.cpp
    src/metric_manager.cpp
    src/cnt_metric.cpp
    src/avg_metric.cpp
    src/cpu_utilization_metric.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC src)

find_package(Threads REQUIRED)
target_link_libraries(MetricLogger PRIVATE Threads::Threads)