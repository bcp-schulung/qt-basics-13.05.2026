#include "WeatherDatabase.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>
#include <cmath>

// ─── SQLite NULL ↔ C++ sentinel helpers ──────────────────────────────────────
static inline QVariant dblVar(double v)       { return std::isnan(v) ? QVariant{}  : QVariant(v); }
static inline QVariant intVar(int v)          { return (v == -1)     ? QVariant{}  : QVariant(v); }
static inline double   toReal(const QVariant &v) { return v.isNull() ? qQNaN()     : v.toDouble(); }
static inline int      toInt (const QVariant &v) { return v.isNull() ? -1          : v.toInt();    }

// ─── DDL ─────────────────────────────────────────────────────────────────────
static const char *CREATE_TABLE_SQL = R"SQL(
CREATE TABLE IF NOT EXISTS weather_records (
    id                      INTEGER PRIMARY KEY AUTOINCREMENT,
    date                    TEXT,
    time_utc                TEXT,
    station_id              TEXT,
    station_name            TEXT,
    latitude                REAL,
    longitude               REAL,
    elevation_m             REAL,
    temperature_c           REAL,
    temperature_f           REAL,
    feels_like_c            REAL,
    dew_point_c             REAL,
    humidity_pct            REAL,
    pressure_station_hpa    REAL,
    pressure_sea_level_hpa  REAL,
    pressure_trend          TEXT,
    wind_speed_kmh          REAL,
    wind_direction_deg      REAL,
    wind_direction_cardinal TEXT,
    wind_gust_kmh           REAL,
    visibility_km           REAL,
    cloud_cover_pct         REAL,
    cloud_ceiling_m         REAL,
    precipitation_type      TEXT,
    precipitation_mm        REAL,
    snow_depth_cm           REAL,
    precipitation_prob_pct  REAL,
    uv_index                REAL,
    solar_radiation_wm2     REAL,
    weather_condition       TEXT,
    weather_code            INTEGER,
    air_quality_index       INTEGER,
    pm2_5_ugm3              REAL,
    pm10_ugm3               REAL,
    ozone_ppb               REAL,
    no2_ppb                 REAL,
    co_ppm                  REAL,
    sunrise_utc             TEXT,
    sunset_utc              TEXT,
    daylight_hours          REAL,
    soil_temp_c             REAL,
    soil_moisture_pct       REAL,
    evapotranspiration_mm   REAL
)
)SQL";

static const char *INSERT_SQL = R"SQL(
INSERT INTO weather_records (
    date, time_utc, station_id, station_name,
    latitude, longitude, elevation_m,
    temperature_c, temperature_f, feels_like_c, dew_point_c,
    humidity_pct, pressure_station_hpa, pressure_sea_level_hpa, pressure_trend,
    wind_speed_kmh, wind_direction_deg, wind_direction_cardinal, wind_gust_kmh,
    visibility_km, cloud_cover_pct, cloud_ceiling_m,
    precipitation_type, precipitation_mm, snow_depth_cm, precipitation_prob_pct,
    uv_index, solar_radiation_wm2,
    weather_condition, weather_code,
    air_quality_index, pm2_5_ugm3, pm10_ugm3, ozone_ppb, no2_ppb, co_ppm,
    sunrise_utc, sunset_utc, daylight_hours,
    soil_temp_c, soil_moisture_pct, evapotranspiration_mm
) VALUES (
    :date, :time_utc, :station_id, :station_name,
    :latitude, :longitude, :elevation_m,
    :temperature_c, :temperature_f, :feels_like_c, :dew_point_c,
    :humidity_pct, :pressure_station_hpa, :pressure_sea_level_hpa, :pressure_trend,
    :wind_speed_kmh, :wind_direction_deg, :wind_direction_cardinal, :wind_gust_kmh,
    :visibility_km, :cloud_cover_pct, :cloud_ceiling_m,
    :precipitation_type, :precipitation_mm, :snow_depth_cm, :precipitation_prob_pct,
    :uv_index, :solar_radiation_wm2,
    :weather_condition, :weather_code,
    :air_quality_index, :pm2_5_ugm3, :pm10_ugm3, :ozone_ppb, :no2_ppb, :co_ppm,
    :sunrise_utc, :sunset_utc, :daylight_hours,
    :soil_temp_c, :soil_moisture_pct, :evapotranspiration_mm
)
)SQL";

// ─── WeatherDatabaseWorker ────────────────────────────────────────────────────

WeatherDatabaseWorker::WeatherDatabaseWorker(const QString &dbPath, QObject *parent)
    : QObject(parent)
    , m_dbPath(dbPath)
{
    // Each instance gets a unique connection name so multiple workers or
    // test instances never share a QSqlDatabase handle.
    m_connectionName = QStringLiteral("WeatherDB_%1")
                           .arg(reinterpret_cast<quintptr>(this));
}

WeatherDatabaseWorker::~WeatherDatabaseWorker()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::database(m_connectionName).close();
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

// Called from inside the worker thread (connected to QThread::started).
bool WeatherDatabaseWorker::openDatabase()
{
    // Ensure the parent directory exists before SQLite tries to create the file.
    QDir dir = QFileInfo(m_dbPath).absoluteDir();
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(m_dbPath);

    if (!db.open()) {
        emit errorOccurred(tr("Cannot open database: %1").arg(db.lastError().text()));
        return false;
    }

    // Write-Ahead Logging gives better concurrent read/write throughput.
    QSqlQuery pragma(db);
    pragma.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    pragma.exec(QStringLiteral("PRAGMA synchronous=NORMAL"));
    return true;
}

bool WeatherDatabaseWorker::createSchema()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    if (!q.exec(QString::fromLatin1(CREATE_TABLE_SQL))) {
        emit errorOccurred(tr("Schema creation failed: %1").arg(q.lastError().text()));
        return false;
    }
    return true;
}

void WeatherDatabaseWorker::initAndLoad()
{
    if (!openDatabase())
        return;
    if (!createSchema())
        return;

    emit loadCompleted(loadAll());
}

QVector<WeatherRecord> WeatherDatabaseWorker::loadAll()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    q.exec(QStringLiteral("SELECT * FROM weather_records ORDER BY id"));

    QVector<WeatherRecord> records;
    records.reserve(1500);

    while (q.next()) {
        WeatherRecord r;
        int c = 1; // column 0 is the autoincrement id — skip it
        r.date                    = q.value(c++).toString();
        r.time_utc                = q.value(c++).toString();
        r.station_id              = q.value(c++).toString();
        r.station_name            = q.value(c++).toString();
        r.latitude                = toReal(q.value(c++));
        r.longitude               = toReal(q.value(c++));
        r.elevation_m             = toReal(q.value(c++));
        r.temperature_c           = toReal(q.value(c++));
        r.temperature_f           = toReal(q.value(c++));
        r.feels_like_c            = toReal(q.value(c++));
        r.dew_point_c             = toReal(q.value(c++));
        r.humidity_pct            = toReal(q.value(c++));
        r.pressure_station_hpa    = toReal(q.value(c++));
        r.pressure_sea_level_hpa  = toReal(q.value(c++));
        r.pressure_trend          = q.value(c++).toString();
        r.wind_speed_kmh          = toReal(q.value(c++));
        r.wind_direction_deg      = toReal(q.value(c++));
        r.wind_direction_cardinal = q.value(c++).toString();
        r.wind_gust_kmh           = toReal(q.value(c++));
        r.visibility_km           = toReal(q.value(c++));
        r.cloud_cover_pct         = toReal(q.value(c++));
        r.cloud_ceiling_m         = toReal(q.value(c++));
        r.precipitation_type      = q.value(c++).toString();
        r.precipitation_mm        = toReal(q.value(c++));
        r.snow_depth_cm           = toReal(q.value(c++));
        r.precipitation_prob_pct  = toReal(q.value(c++));
        r.uv_index                = toReal(q.value(c++));
        r.solar_radiation_wm2     = toReal(q.value(c++));
        r.weather_condition       = q.value(c++).toString();
        r.weather_code            = toInt (q.value(c++));
        r.air_quality_index       = toInt (q.value(c++));
        r.pm2_5_ugm3              = toReal(q.value(c++));
        r.pm10_ugm3               = toReal(q.value(c++));
        r.ozone_ppb               = toReal(q.value(c++));
        r.no2_ppb                 = toReal(q.value(c++));
        r.co_ppm                  = toReal(q.value(c++));
        r.sunrise_utc             = q.value(c++).toString();
        r.sunset_utc              = q.value(c++).toString();
        r.daylight_hours          = toReal(q.value(c++));
        r.soil_temp_c             = toReal(q.value(c++));
        r.soil_moisture_pct       = toReal(q.value(c++));
        r.evapotranspiration_mm   = toReal(q.value(c++));
        records.append(r);
    }
    return records;
}

void WeatherDatabaseWorker::save(const QVector<WeatherRecord> &records)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) {
        emit errorOccurred(tr("Database is not open — cannot save records"));
        return;
    }

    db.transaction();

    // Replace all existing rows.
    QSqlQuery del(db);
    del.exec(QStringLiteral("DELETE FROM weather_records"));

    QSqlQuery q(db);
    if (!q.prepare(QString::fromLatin1(INSERT_SQL))) {
        db.rollback();
        emit errorOccurred(tr("Prepare failed: %1").arg(q.lastError().text()));
        return;
    }

    for (const WeatherRecord &r : records) {
        q.bindValue(QStringLiteral(":date"),                    r.date);
        q.bindValue(QStringLiteral(":time_utc"),                r.time_utc);
        q.bindValue(QStringLiteral(":station_id"),              r.station_id);
        q.bindValue(QStringLiteral(":station_name"),            r.station_name);
        q.bindValue(QStringLiteral(":latitude"),                dblVar(r.latitude));
        q.bindValue(QStringLiteral(":longitude"),               dblVar(r.longitude));
        q.bindValue(QStringLiteral(":elevation_m"),             dblVar(r.elevation_m));
        q.bindValue(QStringLiteral(":temperature_c"),           dblVar(r.temperature_c));
        q.bindValue(QStringLiteral(":temperature_f"),           dblVar(r.temperature_f));
        q.bindValue(QStringLiteral(":feels_like_c"),            dblVar(r.feels_like_c));
        q.bindValue(QStringLiteral(":dew_point_c"),             dblVar(r.dew_point_c));
        q.bindValue(QStringLiteral(":humidity_pct"),            dblVar(r.humidity_pct));
        q.bindValue(QStringLiteral(":pressure_station_hpa"),    dblVar(r.pressure_station_hpa));
        q.bindValue(QStringLiteral(":pressure_sea_level_hpa"),  dblVar(r.pressure_sea_level_hpa));
        q.bindValue(QStringLiteral(":pressure_trend"),          r.pressure_trend);
        q.bindValue(QStringLiteral(":wind_speed_kmh"),          dblVar(r.wind_speed_kmh));
        q.bindValue(QStringLiteral(":wind_direction_deg"),      dblVar(r.wind_direction_deg));
        q.bindValue(QStringLiteral(":wind_direction_cardinal"), r.wind_direction_cardinal);
        q.bindValue(QStringLiteral(":wind_gust_kmh"),           dblVar(r.wind_gust_kmh));
        q.bindValue(QStringLiteral(":visibility_km"),           dblVar(r.visibility_km));
        q.bindValue(QStringLiteral(":cloud_cover_pct"),         dblVar(r.cloud_cover_pct));
        q.bindValue(QStringLiteral(":cloud_ceiling_m"),         dblVar(r.cloud_ceiling_m));
        q.bindValue(QStringLiteral(":precipitation_type"),      r.precipitation_type);
        q.bindValue(QStringLiteral(":precipitation_mm"),        dblVar(r.precipitation_mm));
        q.bindValue(QStringLiteral(":snow_depth_cm"),           dblVar(r.snow_depth_cm));
        q.bindValue(QStringLiteral(":precipitation_prob_pct"),  dblVar(r.precipitation_prob_pct));
        q.bindValue(QStringLiteral(":uv_index"),                dblVar(r.uv_index));
        q.bindValue(QStringLiteral(":solar_radiation_wm2"),     dblVar(r.solar_radiation_wm2));
        q.bindValue(QStringLiteral(":weather_condition"),       r.weather_condition);
        q.bindValue(QStringLiteral(":weather_code"),            intVar(r.weather_code));
        q.bindValue(QStringLiteral(":air_quality_index"),       intVar(r.air_quality_index));
        q.bindValue(QStringLiteral(":pm2_5_ugm3"),              dblVar(r.pm2_5_ugm3));
        q.bindValue(QStringLiteral(":pm10_ugm3"),               dblVar(r.pm10_ugm3));
        q.bindValue(QStringLiteral(":ozone_ppb"),               dblVar(r.ozone_ppb));
        q.bindValue(QStringLiteral(":no2_ppb"),                 dblVar(r.no2_ppb));
        q.bindValue(QStringLiteral(":co_ppm"),                  dblVar(r.co_ppm));
        q.bindValue(QStringLiteral(":sunrise_utc"),             r.sunrise_utc);
        q.bindValue(QStringLiteral(":sunset_utc"),              r.sunset_utc);
        q.bindValue(QStringLiteral(":daylight_hours"),          dblVar(r.daylight_hours));
        q.bindValue(QStringLiteral(":soil_temp_c"),             dblVar(r.soil_temp_c));
        q.bindValue(QStringLiteral(":soil_moisture_pct"),       dblVar(r.soil_moisture_pct));
        q.bindValue(QStringLiteral(":evapotranspiration_mm"),   dblVar(r.evapotranspiration_mm));

        if (!q.exec()) {
            db.rollback();
            emit errorOccurred(tr("Insert failed: %1").arg(q.lastError().text()));
            return;
        }
    }

    if (!db.commit()) {
        // db.rollback();
        emit errorOccurred(tr("Commit failed: %1").arg(db.lastError().text()));
        return;
    }

    emit saveCompleted(records.size());
}

void WeatherDatabaseWorker::clearCache()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) {
        emit errorOccurred(tr("Database is not open — cannot clear cache"));
        return;
    }

    QSqlQuery q(db);
    if (!q.exec(QStringLiteral("DELETE FROM weather_records"))) {
        emit errorOccurred(tr("Clear cache failed: %1").arg(q.lastError().text()));
        return;
    }

    // Reclaim unused pages so the file actually shrinks on disk.
    q.exec(QStringLiteral("VACUUM"));

    emit cacheCleared();
}
