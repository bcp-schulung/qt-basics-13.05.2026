#pragma once

#include <QWidget>
#include "WeatherRecord.h"

// ─── TemperatureBandWidget ────────────────────────────────────────────────────
// Draws three solid rectangles side-by-side (no gap) using QPainter.
// Band definitions (fixed thresholds):
//   Low  : temperature_c < 0  °C  → Blue
//   Mid  : 0 °C ≤ temperature_c < 20 °C  → Yellow
//   High : temperature_c ≥ 20 °C  → Red
// Width of each band is proportional to the number of records in that band.
// Only the outer left/right edges are rounded; internal borders are straight.
// A temperature-range label is drawn centred inside each band.
class TemperatureBandWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TemperatureBandWidget(QWidget *parent = nullptr);

    // Feed the widget new data; triggers a repaint.
    void setRecords(const QVector<WeatherRecord> &records);

    QSize sizeHint()        const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // Thresholds
    static constexpr double LOW_THRESHOLD  =  0.0;  // < 0 °C  → low
    static constexpr double MID_THRESHOLD  = 20.0;  // < 20 °C → mid, else high

    // Per-band record counts and temperature extremes
    int    m_lowCount  = 0;
    int    m_midCount  = 0;
    int    m_highCount = 0;

    double m_lowMin  = 0.0, m_lowMax  = 0.0;
    double m_midMin  = 0.0, m_midMax  = 0.0;
    double m_highMin = 0.0, m_highMax = 0.0;
};
