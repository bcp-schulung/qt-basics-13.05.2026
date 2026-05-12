#include "TemperatureAreaChart.h"

#include <QtCharts/QChartView>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

#include <QVBoxLayout>
#include <QMap>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QLinearGradient>
#include <QPen>
#include <cmath>
#include <limits>

TemperatureAreaChart::TemperatureAreaChart(QWidget *parent)
    : QWidget(parent)
{
    auto *chart = new QChart;
    chart->setTitle("Daily Temperature Range");
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartView = new QChartView(chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chartView);
}

void TemperatureAreaChart::setRecords(const QVector<WeatherRecord> &records)
{
    // ── 1. Aggregate temperature_c by date ────────────────────────────────────
    struct DayStats { double lo; double hi; };
    QMap<QDate, DayStats> dayMap;

    for (const WeatherRecord &r : records) {
        if (std::isnan(r.temperature_c)) continue;

        QDate d = QDate::fromString(r.date, "yyyy-MM-dd");
        if (!d.isValid()) continue;

        auto it = dayMap.find(d);
        if (it == dayMap.end()) {
            dayMap.insert(d, {r.temperature_c, r.temperature_c});
        } else {
            it->lo = std::min(it->lo, r.temperature_c);
            it->hi = std::max(it->hi, r.temperature_c);
        }
    }

    // ── 2. Build upper (max) and lower (min) line series ──────────────────────
    auto *upperSeries = new QLineSeries;
    upperSeries->setName("Daily High (°C)");

    auto *lowerSeries = new QLineSeries;
    lowerSeries->setName("Daily Low (°C)");

    for (auto it = dayMap.cbegin(); it != dayMap.cend(); ++it) {
        // Represent each day as noon UTC to avoid DST shifts
        qint64 ms = QDateTime(it.key(), QTime(12, 0, 0), Qt::UTC).toMSecsSinceEpoch();
        upperSeries->append(ms, it.value().hi);
        lowerSeries->append(ms, it.value().lo);
    }

    // ── 3. Create area series between upper and lower ─────────────────────────
    auto *areaSeries = new QAreaSeries(upperSeries, lowerSeries);
    areaSeries->setName("Temperature Range");

    // Upper border: warm orange-red
    QPen upperPen(QColor(0xE53935));
    upperPen.setWidth(2);
    upperSeries->setPen(upperPen);

    // Lower border: cool blue
    QPen lowerPen(QColor(0x1E88E5));
    lowerPen.setWidth(2);
    lowerSeries->setPen(lowerPen);

    // Area fill: gradient from warm (top = high) to cool (bottom = low)
    QLinearGradient gradient(0, 0, 0, 1);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient.setColorAt(0.0, QColor(0xE5, 0x39, 0x35, 160));  // red at top (hot)
    gradient.setColorAt(1.0, QColor(0x1E, 0x88, 0xE5, 160));  // blue at bottom (cold)
    areaSeries->setBrush(gradient);

    QPen areaPen(Qt::NoPen);
    areaSeries->setPen(areaPen);

    // ── 4. Clear previous series and axes, then re-populate ───────────────────
    QChart *chart = m_chartView->chart();
    chart->removeAllSeries();
    const auto oldAxes = chart->axes();
    for (QAbstractAxis *axis : oldAxes)
        chart->removeAxis(axis);

    chart->addSeries(areaSeries);

    // ── 5. Configure axes ─────────────────────────────────────────────────────
    auto *axisX = new QDateTimeAxis;
    axisX->setFormat("MMM d");
    axisX->setTitleText("Date");
    axisX->setTickCount(10);
    axisX->setLabelsAngle(-45);

    auto *axisY = new QValueAxis;
    axisY->setTitleText("Temperature (°C)");
    axisY->setLabelFormat("%.1f");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    areaSeries->attachAxis(axisX);
    areaSeries->attachAxis(axisY);
}
