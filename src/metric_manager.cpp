#include "metric_manager.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <chrono>

MetricManager::~MetricManager()
{
    stop();
}

void MetricManager::writeLoop()
{
    while (is_running_)
    {
        std::unique_lock<std::mutex> lock(thread_control_mutex_);

        // waiting for the time interval or the stop signal
        if (cv_.wait_for(lock, interval_, [this]
                         { return !is_running_; }))
        {
            break;
        }
        lock.unlock();
        flush();
    }
}
void MetricManager::flush()
{
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    if (metrics_.empty())
        return;

    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm time_info = {};

    // cross-platform selection of a thread-safe function
#if defined(_WIN32)
    localtime_s(&time_info, &now_c);
#else
    localtime_r(&now_c, &time_info);
#endif

    std::stringstream ss;
    ss << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setw(3) << std::setfill('0') << ms.count();

    for (const auto &metric : metrics_)
    {
        const std::string &name = metric->getName();

        // enclose name in quotation marks for better print.
        ss << ' ' << "\"" + name + "\"" << ' ' << metric->getValue();
    }

    ss << std::endl;

    if (file_stream_.is_open())
    {
        file_stream_ << ss.str();
        file_stream_.flush();
    }
    else
    {
        std::cerr << "Warning: File is not open." << std::endl;
    }

    for (const auto &metric : metrics_)
    {
        metric->reset();
    }
}

MetricManager &MetricManager::getInstance()
{
    static MetricManager instance;
    return instance;
}

void MetricManager::start(const std::string &filepath, std::chrono::milliseconds interval)
{
    if (is_running_)
        return;

    filepath_ = filepath;
    interval_ = interval;

    file_stream_.open(filepath_, std::ios::out | std::ios::app);
    if (!file_stream_.is_open())
    {
        throw std::runtime_error("Could not open file: " + filepath_);
    }

    is_running_ = true;
    write_thread_ = std::thread(&MetricManager::writeLoop, this);
}

void MetricManager::stop()
{
    if (!is_running_)
        return;
    // create scope to unlock the mutex automatically
    {
        std::lock_guard<std::mutex> lock(thread_control_mutex_);
        is_running_ = false;
    }

    cv_.notify_one();
    if (write_thread_.joinable())
    {
        write_thread_.join();
    }

    flush();

    if (file_stream_.is_open())
    {
        file_stream_.close();
    }
}