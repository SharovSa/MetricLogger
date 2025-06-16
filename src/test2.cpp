#include "metric_manager.h"
#include <iostream>
#include "avg_metric.h"
#include "cnt_metric.h"
#include "cpu_utilization_metric.h"
#include <random>
#include <cmath>

std::atomic<bool> g_stop_load(false);

// function to load cpu cores
void cpu_load_worker()
{
    volatile double result = 1.23;
    while (!g_stop_load)
    {
        result = sqrt(result) * sin(result) + cos(result) * tan(result);
    }
}

int main()
{
    std::cout << "Start metric simulation 2 \n";

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads > 1)
    {
        num_threads -= 1;
    }
    if (num_threads == 0)
    {
        num_threads = 1;
    }

    auto &mm = MetricManager::getInstance();
    auto http_metric = mm.addMetric<CntMetric>("HTTP requests RPS");

    mm.start("metrics.log", std::chrono::seconds(2));

    std::cout << "Set logger interval to " << mm.getInterval().count() / 1000 << " second\n";

    unsigned int seed = 42;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<unsigned long long> http_metric_gen(10, 1000);

    std::cout << "Start logging only HTTP metric for 5 seconds with metric values:\n";

    auto finish_time = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < finish_time)
    {
        auto http_metric_value = http_metric_gen(gen);
        http_metric->add(http_metric_value);
        std::cout << "HTTP metric value: " << http_metric_value << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto cpu_metric = mm.addMetric<CpuUtilizationMetric>("CPU");
    mm.setInterval(std::chrono::seconds(1));
    std::cout << "Add CPU metric to logger and set time interval to " << mm.getInterval().count() / 1000 << " seconds\n";
    std::cout << "Starting " << num_threads << " CPU load threads..." << std::endl;
    std::vector<std::thread> load_threads;
    for (unsigned int i = 0; i < num_threads; ++i)
    {
        load_threads.emplace_back(cpu_load_worker);
    }

    finish_time = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < finish_time)
    {
        auto http_metric_value = http_metric_gen(gen);
        http_metric->add(http_metric_value);
        std::cout << "HTTP metric value: " << http_metric_value << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // stop workers
    g_stop_load = true;
    for (auto &t : load_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    mm.stop();

    std::cout << "Simulation is finished. Check metric.log file.\n";
    return 0;
}