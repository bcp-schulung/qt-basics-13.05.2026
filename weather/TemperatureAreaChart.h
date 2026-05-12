#pragma once

#include <QWidget>
#include <QVector>
#include "WeatherRecord.h"

QT_FORWARD_DECLARE_CLASS(QChartView)

// ─── TemperatureAreaChart ────────────────────────────────────────────────────────
// A QWidget that renders a QtCharts area chart showing the daily temperature range
// (highest and lowest temperature_c) across all loaded WeatherRecords.
//
// X-axis : dates (QDateTimeAxis, formatted "MMM d")
// Y-axis : temperature in °C (QValueAxis)
// Area   : gradient fill between the upper (max) and lower (min) daily series
//
// Usage:
//   chart->setRecords(model->allRecords());   // call after CSV load
class TemperatureAreaChart : public QWidget
{
    Q_OBJECT

public:
    explicit TemperatureAreaChart(QWidget *parent = nullptr);

    // Aggregates records by date to compute daily min/max, then redraws the chart.
    // Safe to call multiple times (clears previous data each time).
    void setRecords(const QVector<WeatherRecord> &records);

private:
    QChartView *m_chartView;
};
