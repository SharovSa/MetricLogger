#pragma once
#include <string>

/**
 * @brief Interface for metrics.
 */
class IMetric
{
public:
    virtual ~IMetric() = default;

    /**
     * @brief Return the name of the metric.
     */
    virtual const std::string &getName() const = 0;

    /**
     * @brief Return the metric value.
     */
    virtual const std::string getValue() = 0;

    /**
     * @brief Reset the metric value.
     */
    virtual void reset() = 0;
};