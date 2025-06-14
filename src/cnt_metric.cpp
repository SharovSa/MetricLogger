#include "cnt_metric.h"

CntMetric::CntMetric(std::string name) : name_(std::move(name)) {}

void CntMetric::add(unsigned long long value)
{
    cnt_ += value;
}

const std::string &CntMetric::getName() const
{
    return name_;
}

std::string CntMetric::getValue() const
{
    return std::to_string(cnt_.load());
}

void CntMetric::reset()
{
    cnt_ = 0;
}