#pragma once
#include <vector>
#include <fstream>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "imetric.h"

/**
 * @brief A Singleton class for managing metrics.
 *
 * This class creates and registers metrics and logs them to a file with a specified frequency.
 */
class MetricManager
{
private:
    std::atomic<bool> is_running_{false};
    std::atomic<bool> is_file_error_{false};
    std::atomic<long long> interval_ms_;
    std::string filepath_;
    std::ofstream file_stream_;

    std::thread write_thread_;        ///< Thread for periodic writing.
    std::mutex thread_control_mutex_; ///< Mutex for controlling the writer thread.
    std::condition_variable cv_;      ///< Condition variable for thread control.

    std::vector<std::unique_ptr<IMetric>> metrics_;
    std::mutex metrics_mutex_; ///< Mutex for protecting access to metrics.

    MetricManager() = default;
    ~MetricManager();
    void writeLoop();
    void flush();

public:
    /**
     * @brief Returns an instance of the MetricManager.
     */
    static MetricManager &getInstance();

    MetricManager(const MetricManager &) = delete;
    MetricManager &operator=(const MetricManager &) = delete;
    MetricManager(MetricManager &&) = delete;
    MetricManager &operator=(MetricManager &&) = delete;

    template <typename T, typename... Args>
    T *addMetric(Args &...args);

    /**
     * @brief Starts the metric manager.
     *
     * Opens the output file and begins periodic metric flushing.
     *
     * @param filepath Path to the output file.
     * @param interval Time interval between flushes.
     */
    void start(const std::string &filepath, std::chrono::milliseconds interval);

    /**
     * @brief Stops the metric manager.
     *
     * Stops the writing thread and closes the file.
     */
    void stop();

    /**
     * @brief Set the time interval between logging.
     *
     * @param new_interval new time interval
     */
    void setInterval(std::chrono::milliseconds new_interval);

    /**
     * @brief Get the time interval between logging.
     *
     * @return time interval, ms
     */
    std::chrono::milliseconds getInterval() const;

    /**
     * @brief Delete all metrics
     */
    void clearMetrics();
};

/**
 * @brief Register a metric.
 *
 * @tparam T Metric type.
 * @tparam Args Types of constructor arguments.
 * @param args Arguments to construct the metric.
 * @return Pointer to the created metric.
 */
template <typename T, typename... Args>
T *MetricManager::addMetric(Args &...args)
{
    static_assert(std::is_base_of<IMetric, T>::value, "T must be a descendant of IMetric");
    auto metric = std::make_unique<T>(std::forward<Args>(args)...);
    T *ptr = metric.get();
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_.push_back(std::move(metric));
    return ptr;
}