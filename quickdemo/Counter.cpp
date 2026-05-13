#include "Counter.h"

Counter::Counter(QObject *parent)
    : QObject(parent)
{}

int Counter::count() const
{
    return m_count;
}

void Counter::setCount(int value)
{
    if (m_count == value)
        return;
    m_count = value;
    emit countChanged(m_count);
    checkThreshold();
}

int Counter::threshold() const
{
    return m_threshold;
}

void Counter::setThreshold(int value)
{
    if (m_threshold == value)
        return;
    m_threshold = value;
    emit thresholdChanged(m_threshold);
}

void Counter::increment()
{
    setCount(m_count + 1);
}

void Counter::decrement()
{
    setCount(m_count - 1);
}

void Counter::reset()
{
    setCount(0);
}

QString Counter::summary() const
{
    return QString("Count is %1 (threshold: %2)").arg(m_count).arg(m_threshold);
}

void Counter::checkThreshold()
{
    if (m_count == m_threshold)
        emit thresholdReached(m_count);
}
