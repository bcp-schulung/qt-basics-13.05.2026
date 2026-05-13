#include <QTest>
#include <cmath>

#include "WeatherRecord.h"

class TestWeatherRecord : public QObject
{
    Q_OBJECT

private slots:
    void defaultDoublesSentinel();
    void defaultIntsSentinel();
    void defaultStringsEmpty();
    void canAssignAndReadFields();
};

// Every double field must be NaN by default (= "no data").
void TestWeatherRecord::defaultDoublesSentinel()
{
    WeatherRecord r;
    QVERIFY(std::isnan(r.latitude));
    QVERIFY(std::isnan(r.longitude));
    QVERIFY(std::isnan(r.elevation_m));
    QVERIFY(std::isnan(r.temperature_c));
    QVERIFY(std::isnan(r.temperature_f));
    QVERIFY(std::isnan(r.feels_like_c));
    QVERIFY(std::isnan(r.dew_point_c));
    QVERIFY(std::isnan(r.humidity_pct));
    QVERIFY(std::isnan(r.pressure_station_hpa));
    QVERIFY(std::isnan(r.pressure_sea_level_hpa));
    QVERIFY(std::isnan(r.wind_speed_kmh));
    QVERIFY(std::isnan(r.wind_direction_deg));
    QVERIFY(std::isnan(r.wind_gust_kmh));
    QVERIFY(std::isnan(r.visibility_km));
    QVERIFY(std::isnan(r.cloud_cover_pct));
    QVERIFY(std::isnan(r.cloud_ceiling_m));
    QVERIFY(std::isnan(r.precipitation_mm));
    QVERIFY(std::isnan(r.snow_depth_cm));
    QVERIFY(std::isnan(r.precipitation_prob_pct));
    QVERIFY(std::isnan(r.uv_index));
    QVERIFY(std::isnan(r.solar_radiation_wm2));
    QVERIFY(std::isnan(r.pm2_5_ugm3));
    QVERIFY(std::isnan(r.pm10_ugm3));
    QVERIFY(std::isnan(r.ozone_ppb));
    QVERIFY(std::isnan(r.no2_ppb));
    QVERIFY(std::isnan(r.co_ppm));
    QVERIFY(std::isnan(r.daylight_hours));
    QVERIFY(std::isnan(r.soil_temp_c));
    QVERIFY(std::isnan(r.soil_moisture_pct));
    QVERIFY(std::isnan(r.evapotranspiration_mm));
}

// Integer fields use -1 as the "missing" sentinel.
void TestWeatherRecord::defaultIntsSentinel()
{
    WeatherRecord r;
    QCOMPARE(r.weather_code,       -1);
    QCOMPARE(r.air_quality_index,  -1);
}

// All QString fields must be empty by default.
void TestWeatherRecord::defaultStringsEmpty()
{
    WeatherRecord r;
    QVERIFY(r.date.isEmpty());
    QVERIFY(r.time_utc.isEmpty());
    QVERIFY(r.station_id.isEmpty());
    QVERIFY(r.station_name.isEmpty());
    QVERIFY(r.pressure_trend.isEmpty());
    QVERIFY(r.wind_direction_cardinal.isEmpty());
    QVERIFY(r.precipitation_type.isEmpty());
    QVERIFY(r.weather_condition.isEmpty());
    QVERIFY(r.sunrise_utc.isEmpty());
    QVERIFY(r.sunset_utc.isEmpty());
}

// Assigning values must be reflected back correctly.
void TestWeatherRecord::canAssignAndReadFields()
{
    WeatherRecord r;
    r.date              = QStringLiteral("2024-07-15");
    r.station_name      = QStringLiteral("Vienna");
    r.temperature_c     = 28.4;
    r.humidity_pct      = 55.0;
    r.weather_code      = 800;
    r.air_quality_index = 42;
    r.pressure_trend    = QStringLiteral("Rising");

    QCOMPARE(r.date,              QStringLiteral("2024-07-15"));
    QCOMPARE(r.station_name,      QStringLiteral("Vienna"));
    QCOMPARE(r.temperature_c,     28.4);
    QCOMPARE(r.humidity_pct,      55.0);
    QCOMPARE(r.weather_code,      800);
    QCOMPARE(r.air_quality_index, 42);
    QCOMPARE(r.pressure_trend,    QStringLiteral("Rising"));
}

QTEST_MAIN(TestWeatherRecord)
#include "tst_WeatherRecord.moc"
