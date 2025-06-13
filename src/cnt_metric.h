#pragma once
#include "imetric.h"
#include <atomic>

/**
 * @brief Counter metric class.
 *
 * This metric contains the sum of the values passed to it.
 */
class CntMetric : public IMetric
{
private:
    std::atomic<unsigned long long> cnt_{0};
    std::string name_;

public:
    CntMetric(std::string name);

    /**
     * @brief Add a value to the counter.
     * @param value Value to add.
     */
    void add(unsigned long long value = 1);

    const std::string &getName() const override;
    const std::string getValue() override;
    void reset() override;
};