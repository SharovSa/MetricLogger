#include "cpu_utilization_metric.h"
#include <stdexcept>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <thread>

#if defined(_WIN32)
#include <windows.h>
#else // Linux/POSIX
#include <fstream>
#include <string>
#include <unistd.h>
#endif

CpuUtilizationMetric::CpuUtilizationMetric(std::string name) : name_(std::move(name))
{
    prev_times_ = getAllCoreTimes();
    usages_.resize(prev_times_.size(), 0.0);
    core_count_ = std::thread::hardware_concurrency();
}

const std::string &CpuUtilizationMetric::getName() const
{
    return name_;
}

std::string CpuUtilizationMetric::getValue() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    update();
    double total_usage = std::accumulate(usages_.begin(), usages_.end(), 0.0);

#if defined(_WIN32)
    total_usage *= core_count_;
#endif

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << total_usage;
    return ss.str();
}

void CpuUtilizationMetric::update() const
{
    std::vector<CoreTimes> current_times = getAllCoreTimes();

    if (current_times.size() != prev_times_.size())
    {
        prev_times_ = current_times;
        usages_.assign(usages_.size(), 0.0);
        return;
    }

    for (size_t i = 0; i < current_times.size(); ++i)
    {
        const auto &prev = prev_times_[i];
        const auto &curr = current_times[i];

        uint64_t prev_total = prev.userTime + prev.systemTime + prev.idleTime;
        uint64_t curr_total = curr.userTime + curr.systemTime + curr.idleTime;
        uint64_t total_diff = curr_total - prev_total;

        if (total_diff == 0)
        {
            usages_[i] = 0.0;
            continue;
        }

        uint64_t idle_diff = curr.idleTime - prev.idleTime;
        usages_[i] = 1.0 - (static_cast<double>(idle_diff) / total_diff);

        // rounding inaccuracies of float
        if (usages_[i] < 0.0)
            usages_[i] = 0.0;
        if (usages_[i] > 1.0)
            usages_[i] = 1.0;
    }

    prev_times_ = current_times;
}

std::vector<CpuUtilizationMetric::CoreTimes> CpuUtilizationMetric::getAllCoreTimes()
{
    std::vector<CoreTimes> times;

#if defined(_WIN32) // Windows
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
    {
        throw std::runtime_error("Failed to get system times.");
    }

    auto to_uint64 = [](const FILETIME &ft)
    {
        return (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    };

    times.resize(1);
    times[0].idleTime = to_uint64(idleTime);
    times[0].systemTime = to_uint64(kernelTime) - to_uint64(idleTime);
    times[0].userTime = to_uint64(userTime);

#else // Linux
    std::ifstream stat_file("/proc/stat");
    if (!stat_file.is_open())
    {
        throw std::runtime_error("Failed to open /proc/stat on Linux.");
    }
    std::string line;
    while (std::getline(stat_file, line))
    {
        if (line.rfind("cpu", 0) == 0 && line.rfind("cpu ", 0) != 0)
        {
            std::stringstream ss(line);
            std::string cpu_label;
            ss >> cpu_label;

            CoreTimes core_time;
            uint64_t nice, iowait, irq, softirq, steal, guest, guest_nice;
            ss >> core_time.userTime >> nice >> core_time.systemTime >> core_time.idleTime >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

            core_time.userTime += nice;
            core_time.systemTime += irq + softirq;

            times.push_back(core_time);
        }
    }
#endif
    return times;
}