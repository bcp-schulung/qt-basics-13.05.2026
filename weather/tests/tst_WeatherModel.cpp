#include <QTest>
#include <QTemporaryFile>
#include <QTextStream>
#include <cmath>

#include "WeatherModel.h"
#include "WeatherRecord.h"

// ─── Helper ──────────────────────────────────────────────────────────────────

static WeatherRecord makeRecord(const QString &date,
                                const QString &station,
                                double         tempC)
{
    WeatherRecord r;
    r.date              = date;
    r.time_utc          = QStringLiteral("12:00");
    r.station_id        = QStringLiteral("S01");
    r.station_name      = station;
    r.temperature_c     = tempC;
    r.temperature_f     = tempC * 9.0 / 5.0 + 32.0;
    r.humidity_pct      = 65.0;
    r.weather_code      = 800;
    r.air_quality_index = 30;
    // cloud_ceiling_m intentionally left NaN (default)
    return r;
}

// Build a valid 42-column CSV data row.
static QString csvRow(const QString &date, const QString &station, double tempC)
{
    QStringList cols(COL_COUNT);    // all empty by default
    cols[COL_DATE]          = date;
    cols[COL_TIME_UTC]      = QStringLiteral("08:00");
    cols[COL_STATION_ID]    = QStringLiteral("T01");
    cols[COL_STATION_NAME]  = station;
    cols[COL_LATITUDE]      = QStringLiteral("48.2000");
    cols[COL_LONGITUDE]     = QStringLiteral("16.3667");
    cols[COL_ELEVATION_M]   = QStringLiteral("171");
    cols[COL_TEMPERATURE_C] = QString::number(tempC, 'f', 1);
    cols[COL_TEMPERATURE_F] = QString::number(tempC * 9.0 / 5.0 + 32.0, 'f', 1);
    return cols.join(QLatin1Char(','));
}

// ─── Test class ──────────────────────────────────────────────────────────────

class TestWeatherModel : public QObject
{
    Q_OBJECT

private slots:
    void emptyModel_rowCountZero();
    void emptyModel_columnCount();
    void setRecords_updatesRowCount();
    void data_displayRole_stringColumn();
    void data_displayRole_numericColumn();
    void data_displayRole_nanShowsDash();
    void data_userRole_numericReturnsDouble();
    void data_invalidIndex_returnsInvalid();
    void headerData_columnNames();
    void clear_resetsModel();
    void loadFromFile_nonexistent_returnsFalse();
    void loadFromFile_validCsv_loadsRows();
    void loadFromFile_validCsv_correctValues();
};

void TestWeatherModel::emptyModel_rowCountZero()
{
    WeatherModel m;
    QCOMPARE(m.rowCount(), 0);
}

void TestWeatherModel::emptyModel_columnCount()
{
    WeatherModel m;
    // columnCount must equal the enum sentinel COL_COUNT (42 columns).
    QCOMPARE(m.columnCount(), static_cast<int>(COL_COUNT));
}

void TestWeatherModel::setRecords_updatesRowCount()
{
    WeatherModel m;
    QVector<WeatherRecord> records;
    records << makeRecord("2024-01-01", "Vienna", 3.0)
            << makeRecord("2024-01-02", "Vienna", 5.5)
            << makeRecord("2024-01-03", "Vienna", 1.2);
    m.setRecords(records);
    QCOMPARE(m.rowCount(), 3);
}

void TestWeatherModel::data_displayRole_stringColumn()
{
    WeatherModel m;
    QVector<WeatherRecord> records;
    records << makeRecord("2024-06-01", "Berlin", 22.0);
    m.setRecords(records);

    QCOMPARE(m.data(m.index(0, COL_DATE),         Qt::DisplayRole).toString(),
             QStringLiteral("2024-06-01"));
    QCOMPARE(m.data(m.index(0, COL_STATION_NAME),  Qt::DisplayRole).toString(),
             QStringLiteral("Berlin"));
    QCOMPARE(m.data(m.index(0, COL_TIME_UTC),      Qt::DisplayRole).toString(),
             QStringLiteral("12:00"));
}

void TestWeatherModel::data_displayRole_numericColumn()
{
    WeatherModel m;
    QVector<WeatherRecord> records;
    records << makeRecord("2024-06-01", "Berlin", 22.3);
    m.setRecords(records);

    // temperature_c rendered with 1 decimal place
    QCOMPARE(m.data(m.index(0, COL_TEMPERATURE_C), Qt::DisplayRole).toString(),
             QStringLiteral("22.3"));
    // humidity_pct rendered with 1 decimal place
    QCOMPARE(m.data(m.index(0, COL_HUMIDITY_PCT),  Qt::DisplayRole).toString(),
             QStringLiteral("65.0"));
}

void TestWeatherModel::data_displayRole_nanShowsDash()
{
    WeatherModel m;
    QVector<WeatherRecord> records;
    records << makeRecord("2024-06-01", "Berlin", 10.0);
    // cloud_ceiling_m is NaN by default; display must be the em dash "—"
    m.setRecords(records);

    QCOMPARE(m.data(m.index(0, COL_CLOUD_CEILING), Qt::DisplayRole).toString(),
             QStringLiteral("—"));
}

void TestWeatherModel::data_userRole_numericReturnsDouble()
{
    WeatherModel m;
    QVector<WeatherRecord> records;
    records << makeRecord("2024-06-01", "Berlin", 15.7);
    m.setRecords(records);

    const QVariant v = m.data(m.index(0, COL_TEMPERATURE_C), Qt::UserRole);
    QVERIFY(v.isValid());
    QCOMPARE(v.toDouble(), 15.7);
}

void TestWeatherModel::data_invalidIndex_returnsInvalid()
{
    WeatherModel m;
    QVector<WeatherRecord> records;
    records << makeRecord("2024-06-01", "Berlin", 5.0);
    m.setRecords(records);

    // Out-of-bounds row / column and a default-constructed QModelIndex
    QVERIFY(!m.data(m.index(-1,  0), Qt::DisplayRole).isValid());
    QVERIFY(!m.data(m.index( 0, -1), Qt::DisplayRole).isValid());
    QVERIFY(!m.data(m.index( 1,  0), Qt::DisplayRole).isValid());
    QVERIFY(!m.data(QModelIndex{},   Qt::DisplayRole).isValid());
}

void TestWeatherModel::headerData_columnNames()
{
    WeatherModel m;
    QCOMPARE(m.headerData(COL_DATE,          Qt::Horizontal, Qt::DisplayRole).toString(),
             QStringLiteral("Date"));
    QCOMPARE(m.headerData(COL_TEMPERATURE_C, Qt::Horizontal, Qt::DisplayRole).toString(),
             QStringLiteral("Temp (°C)"));
    QCOMPARE(m.headerData(COL_HUMIDITY_PCT,  Qt::Horizontal, Qt::DisplayRole).toString(),
             QStringLiteral("Humidity (%)"));
    // Vertical headers and unknown roles must return an invalid QVariant.
    QVERIFY(!m.headerData(0, Qt::Vertical, Qt::DisplayRole).isValid());
}

void TestWeatherModel::clear_resetsModel()
{
    WeatherModel m;
    QVector<WeatherRecord> records;
    records << makeRecord("2024-01-01", "Rome", 18.0);
    m.setRecords(records);
    QCOMPARE(m.rowCount(), 1);

    m.clear();
    QCOMPARE(m.rowCount(), 0);
}

void TestWeatherModel::loadFromFile_nonexistent_returnsFalse()
{
    WeatherModel m;
    QString error;
    QVERIFY(!m.loadFromFile(QStringLiteral("/tmp/no_such_file_xyz.csv"), &error));
    QVERIFY(!error.isEmpty());
}

void TestWeatherModel::loadFromFile_validCsv_loadsRows()
{
    QTemporaryFile f;
    f.setAutoRemove(true);
    QVERIFY(f.open());
    {
        QTextStream out(&f);
        // Header row — content is irrelevant, it is always skipped.
        out << "header\n";
        out << csvRow("2024-03-10", "Paris",  8.0) << "\n";
        out << csvRow("2024-03-11", "Paris", 10.5) << "\n";
    }
    f.close();

    WeatherModel m;
    QVERIFY(m.loadFromFile(f.fileName()));
    QCOMPARE(m.rowCount(), 2);
}

void TestWeatherModel::loadFromFile_validCsv_correctValues()
{
    QTemporaryFile f;
    f.setAutoRemove(true);
    QVERIFY(f.open());
    {
        QTextStream out(&f);
        out << "header\n";
        out << csvRow("2024-05-20", "Madrid", 25.6) << "\n";
    }
    f.close();

    WeatherModel m;
    QVERIFY(m.loadFromFile(f.fileName()));
    QCOMPARE(m.rowCount(), 1);

    const WeatherRecord &r = m.record(0);
    QCOMPARE(r.date,          QStringLiteral("2024-05-20"));
    QCOMPARE(r.station_name,  QStringLiteral("Madrid"));
    QCOMPARE(r.temperature_c, 25.6);
}

QTEST_MAIN(TestWeatherModel)
#include "tst_WeatherModel.moc"
