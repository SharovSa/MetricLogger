#pragma once
#include "imetric.h"
#include <mutex>

/**
 * @brief Average metric class.
 *
 * This metric contains the average of the values passed to it.
 */
class AvgMetric : public IMetric
{
private:
    double sum_ = 0.0;
    unsigned long long count_ = 0;
    std::string name_;
    mutable std::mutex mutex_;

public:
    AvgMetric(std::string name);

    /**
     * @brief Add a value to the metric.
     * @param value Value to add.
     */
    void add(double value);
    const std::string &getName() const override;
    std::string getValue() const override;
    void reset() override;
};