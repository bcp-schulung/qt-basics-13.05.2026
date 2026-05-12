#include "TemperatureBandWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <cmath>
#include <limits>
#include <algorithm>

TemperatureBandWidget::TemperatureBandWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(46);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

// ─── Data ─────────────────────────────────────────────────────────────────────

void TemperatureBandWidget::setRecords(const QVector<WeatherRecord> &records)
{
    m_lowCount  = 0;
    m_midCount  = 0;
    m_highCount = 0;

    double lowMin  =  std::numeric_limits<double>::infinity();
    double lowMax  = -std::numeric_limits<double>::infinity();
    double midMin  =  std::numeric_limits<double>::infinity();
    double midMax  = -std::numeric_limits<double>::infinity();
    double highMin =  std::numeric_limits<double>::infinity();
    double highMax = -std::numeric_limits<double>::infinity();

    for (const WeatherRecord &r : records) {
        const double t = r.temperature_c;
        if (std::isnan(t)) continue;

        if (t < LOW_THRESHOLD) {
            ++m_lowCount;
            lowMin = std::min(lowMin, t);
            lowMax = std::max(lowMax, t);
        } else if (t < MID_THRESHOLD) {
            ++m_midCount;
            midMin = std::min(midMin, t);
            midMax = std::max(midMax, t);
        } else {
            ++m_highCount;
            highMin = std::min(highMin, t);
            highMax = std::max(highMax, t);
        }
    }

    // Store extremes (fall back to threshold boundary when a band is empty)
    m_lowMin  = std::isfinite(lowMin)  ? lowMin  : LOW_THRESHOLD;
    m_lowMax  = std::isfinite(lowMax)  ? lowMax  : LOW_THRESHOLD;
    m_midMin  = std::isfinite(midMin)  ? midMin  : LOW_THRESHOLD;
    m_midMax  = std::isfinite(midMax)  ? midMax  : MID_THRESHOLD;
    m_highMin = std::isfinite(highMin) ? highMin : MID_THRESHOLD;
    m_highMax = std::isfinite(highMax) ? highMax : MID_THRESHOLD;

    update();
}

// ─── Size hints ───────────────────────────────────────────────────────────────

QSize TemperatureBandWidget::sizeHint() const        { return {400, 56}; }
QSize TemperatureBandWidget::minimumSizeHint() const { return {120, 46}; }

// ─── Painting ─────────────────────────────────────────────────────────────────

void TemperatureBandWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r   = QRectF(rect()).adjusted(1, 1, -1, -1);
    const qreal  rad = 7.0;

    const int total = m_lowCount + m_midCount + m_highCount;

    // ── Empty state ──────────────────────────────────────────────────────────
    if (total == 0) {
        p.setPen(QPen(palette().mid().color(), 1));
        p.setBrush(palette().button());
        p.drawRoundedRect(r, rad, rad);
        p.setPen(palette().buttonText().color());
        p.drawText(r, Qt::AlignCenter, tr("No data loaded"));
        return;
    }

    // ── Compute pixel widths (proportional) ──────────────────────────────────
    const qreal w      = r.width();
    const qreal lowW   = std::round(w * m_lowCount  / total);
    const qreal highW  = std::round(w * m_highCount / total);
    const qreal midW   = w - lowW - highW;

    const QRectF lowRect (r.x(),            r.y(), lowW,  r.height());
    const QRectF midRect (r.x() + lowW,     r.y(), midW,  r.height());
    const QRectF highRect(r.x() + lowW + midW, r.y(), highW, r.height());

    // ── Clip to the outer rounded rectangle ──────────────────────────────────
    // This gives rounded outer edges while keeping internal joins sharp.
    QPainterPath clip;
    clip.addRoundedRect(r, rad, rad);
    p.setClipPath(clip);

    // ── Fill bands ───────────────────────────────────────────────────────────
    const QColor colorLow ("#5B9BD5");   // steel blue
    const QColor colorMid ("#FFD700");   // golden yellow
    const QColor colorHigh("#D94F4F");   // warm red

    if (lowW  > 0) p.fillRect(lowRect,  colorLow);
    if (midW  > 0) p.fillRect(midRect,  colorMid);
    if (highW > 0) p.fillRect(highRect, colorHigh);

    // ── Labels ───────────────────────────────────────────────────────────────
    QFont f = font();
    f.setBold(true);
    p.setFont(f);

    const QFontMetrics fm(f);
    // Minimum pixel width to bother drawing a label (rough: "−9.9°C – 0.0°C")
    const int minLabelWidth = fm.horizontalAdvance(QStringLiteral("−9.9°C–0.0°C")) + 8;

    auto drawBandLabel = [&](const QRectF &bandRect,
                              double minT, double maxT,
                              bool darkText)
    {
        if (bandRect.width() < minLabelWidth) return;
        const QString text = QString("%1°C – %2°C")
                                 .arg(minT, 0, 'f', 1)
                                 .arg(maxT, 0, 'f', 1);
        p.setPen(darkText ? QColor(0x22, 0x22, 0x22) : Qt::white);
        p.drawText(bandRect, Qt::AlignCenter, text);
    };

    if (lowW  > 0) drawBandLabel(lowRect,  m_lowMin,  m_lowMax,  false); // blue  → white text
    if (midW  > 0) drawBandLabel(midRect,  m_midMin,  m_midMax,  true);  // yellow→ dark text
    if (highW > 0) drawBandLabel(highRect, m_highMin, m_highMax, false); // red   → white text

    // ── Outer border (drawn after clipping is removed) ────────────────────────
    p.setClipping(false);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(palette().mid().color(), 1));
    p.drawRoundedRect(r, rad, rad);
}
