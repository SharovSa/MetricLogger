#include "avg_metric.h"

AvgMetric::AvgMetric(std::string name) : name_(std::move(name)) {}

void AvgMetric::add(double value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sum_ += value;
    ++count_;
}

const std::string &AvgMetric::getName() const
{
    return name_;
}

const std::string AvgMetric::getValue()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ == 0)
    {
        return "0.0";
    }
    double avg = sum_ / count_;
    std::string str_avg = std::to_string(avg);
    return str_avg;
}

void AvgMetric::reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    sum_ = 0.0;
    count_ = 0;
}