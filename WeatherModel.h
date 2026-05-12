#pragma once

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QVector>
#include <QColor>
#include "WeatherRecord.h"

// ─── WeatherModel ────────────────────────────────────────────────────────────────
// Custom QAbstractTableModel that owns a flat QVector<WeatherRecord>.
// data() serves:
//   Qt::DisplayRole    – formatted QString for display
//   Qt::UserRole       – raw typed value (double/int/QString) for sort proxy
//   Qt::BackgroundRole – pastel QBrush for temperature / AQI / precip-type cols
//   Qt::ForegroundRole – white text on dark AQI backgrounds
//   Qt::TextAlignmentRole – right-align numerics, left-align strings
class WeatherModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit WeatherModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int      rowCount   (const QModelIndex &parent = {}) const override;
    int      columnCount(const QModelIndex &parent = {}) const override;
    QVariant data       (const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData (int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const override;

    // CSV loading
    bool loadFromFile(const QString &filePath, QString *error = nullptr);
    void clear();

    // Direct access for stats / detail-panel callers
    const WeatherRecord &record(int row) const { return m_records.at(row); }
    int  recordCount() const                   { return m_records.size();  }

private:
    QVector<WeatherRecord> m_records;

    // data() helpers
    QVariant displayData  (const WeatherRecord &r, int col) const;
    QVariant sortData     (const WeatherRecord &r, int col) const;
    QVariant backgroundFor(const WeatherRecord &r, int col) const;
    QVariant foregroundFor(const WeatherRecord &r, int col) const;

    // Per-column background colours
    static QColor temperatureColor(double tempC);   // blue → white → pink/red
    static QColor aqiColor        (int aqi);        // green / yellow / orange / red / purple
    static QColor precipTypeColor (const QString &type);

    // CSV parse helpers
    static double        parseDouble(const QString &s);
    static int           parseInt   (const QString &s);
    static WeatherRecord parseRow   (const QStringList &fields);
};

// ─── WeatherProxyModel ───────────────────────────────────────────────────────────
// Extends QSortFilterProxyModel with:
//   • Multi-column text filter (checks every column's DisplayRole)
//   • Type-aware lessThan() using Qt::UserRole (raw double/int) to sort
//     numerics correctly; NaN / -1 (missing) always sort to the end.
class WeatherProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit WeatherProxyModel(QObject *parent = nullptr);

    void setFilterText(const QString &text);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan        (const QModelIndex &left, const QModelIndex &right) const override;

private:
    QString m_filterText;
};
