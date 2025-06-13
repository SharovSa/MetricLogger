#include "metric_manager.h"
#include <iostream>
#include "avg_metric.h"
#include "cnt_metric.h"
#include <random>

int main()
{
    std::cout << "Start metric simulation \n";
    auto &mm = MetricManager::getInstance();
    auto cpu_metric = mm.registerMetric<AvgMetric>("CPU");
    auto http_metric = mm.registerMetric<CntMetric>("HTTP requests RPS");

    // Start the logger with a 1-second interval
    mm.start("metrics.log", std::chrono::seconds(1));
    unsigned int seed = 42;
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> cpu_metric_gen(0, 2);
    std::uniform_int_distribution<unsigned long long> http_metric_gen(10, 1000);

    std::cout << "Start logging for 10 seconds with metric values:\n";

    auto finish_time = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (std::chrono::steady_clock::now() < finish_time)
    {
        auto cpu_metric_value = cpu_metric_gen(gen);
        auto http_metric_value = http_metric_gen(gen);
        cpu_metric->add(cpu_metric_value);
        http_metric->add(http_metric_value);
        std::cout << "Add CPU metric value: " << cpu_metric_value << " HTTP metric value: " << http_metric_value << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    mm.stop();

    std::cout << "Simulation is finished. Check metric.log file.\n";
    return 0;
}