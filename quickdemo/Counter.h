#pragma once

#include <QObject>
#include <QtQml/qqml.h>

class Counter : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // Exposes "count" as a readable/writable QML property that emits countChanged
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)

    // Read-only property: fires thresholdReached signal when count hits this value
    Q_PROPERTY(int threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)

public:
    explicit Counter(QObject *parent = nullptr);

    int count() const;
    void setCount(int value);

    int threshold() const;
    void setThreshold(int value);

    // Q_INVOKABLE makes these callable directly from QML as counter.increment()
    Q_INVOKABLE void increment();
    Q_INVOKABLE void decrement();
    Q_INVOKABLE void reset();

    // Returns a formatted summary string — shows passing data back to QML
    Q_INVOKABLE QString summary() const;

signals:
    void countChanged(int count);
    void thresholdChanged(int threshold);
    void thresholdReached(int count);

private:
    int m_count = 0;
    int m_threshold = 10;

    void checkThreshold();
};
