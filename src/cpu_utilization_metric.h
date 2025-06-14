#pragma once

#include "imetric.h"
#include <vector>
#include <string>
#include <cstdint>
#include <mutex>

/**
 * @brief CPU utilization metric class.
 *
 * This metric calculates the percentage of load of each CPU core and outputs their sum.
 */
class CpuUtilizationMetric : public IMetric
{
private:
    /**
     * @brief Аuxiliary structure.
     * @param userTime Core work in user mode time
     * @param systemTime Core work in kernel mode time
     * @param idleTime Core downtime
     *
     * This structure stores the operating time of the cores.
     */
    struct CoreTimes
    {
        uint64_t userTime = 0;
        uint64_t systemTime = 0;
        uint64_t idleTime = 0;
    };

    /**
     * @brief Function for getting the work time of the cores.
     *
     * This is a cross-platform function.
     */
    static std::vector<CoreTimes> getAllCoreTimes();

    /**
     * @brief Update a cpu usage
     *
     * Usage of cpu is sum of core usages.
     * Usage of each core = 1 - idleTime / totalTime
     */
    void update() const;

    std::string name_;
    unsigned int core_count_ = 0;
    mutable std::mutex mutex_;
    mutable std::vector<CoreTimes> prev_times_; ///< previous measurements
    mutable std::vector<double> usages_;        ///< Last utilization values

public:
    explicit CpuUtilizationMetric(std::string name);
    const std::string &getName() const override;
    std::string getValue() const override;
    void reset() override {}
};