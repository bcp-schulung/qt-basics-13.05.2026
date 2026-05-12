#include "WeatherModel.h"

#include <QFile>
#include <QTextStream>
#include <QBrush>
#include <QtMath>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════════
//  CSV parse helpers
// ═══════════════════════════════════════════════════════════════════════════════

double WeatherModel::parseDouble(const QString &s)
{
    if (s.isEmpty()) return qQNaN();
    bool ok = false;
    double v = s.toDouble(&ok);
    return ok ? v : qQNaN();
}

int WeatherModel::parseInt(const QString &s)
{
    if (s.isEmpty()) return -1;
    bool ok = false;
    int v = s.toInt(&ok);
    return ok ? v : -1;
}

WeatherRecord WeatherModel::parseRow(const QStringList &f)
{
    // f must have exactly COL_COUNT (42) fields.
    WeatherRecord r;
    r.date                     = f[COL_DATE];
    r.time_utc                 = f[COL_TIME_UTC];
    r.station_id               = f[COL_STATION_ID];
    r.station_name             = f[COL_STATION_NAME];
    r.latitude                 = parseDouble(f[COL_LATITUDE]);
    r.longitude                = parseDouble(f[COL_LONGITUDE]);
    r.elevation_m              = parseDouble(f[COL_ELEVATION_M]);
    r.temperature_c            = parseDouble(f[COL_TEMPERATURE_C]);
    r.temperature_f            = parseDouble(f[COL_TEMPERATURE_F]);
    r.feels_like_c             = parseDouble(f[COL_FEELS_LIKE_C]);
    r.dew_point_c              = parseDouble(f[COL_DEW_POINT_C]);
    r.humidity_pct             = parseDouble(f[COL_HUMIDITY_PCT]);
    r.pressure_station_hpa     = parseDouble(f[COL_PRESSURE_STATION]);
    r.pressure_sea_level_hpa   = parseDouble(f[COL_PRESSURE_SEA_LEVEL]);
    r.pressure_trend           = f[COL_PRESSURE_TREND];
    r.wind_speed_kmh           = parseDouble(f[COL_WIND_SPEED]);
    r.wind_direction_deg       = parseDouble(f[COL_WIND_DIR_DEG]);
    r.wind_direction_cardinal  = f[COL_WIND_DIR_CARDINAL];
    r.wind_gust_kmh            = parseDouble(f[COL_WIND_GUST]);
    r.visibility_km            = parseDouble(f[COL_VISIBILITY]);
    r.cloud_cover_pct          = parseDouble(f[COL_CLOUD_COVER]);
    r.cloud_ceiling_m          = parseDouble(f[COL_CLOUD_CEILING]);  // may be NaN
    r.precipitation_type       = f[COL_PRECIP_TYPE];
    r.precipitation_mm         = parseDouble(f[COL_PRECIP_MM]);
    r.snow_depth_cm            = parseDouble(f[COL_SNOW_DEPTH]);
    r.precipitation_prob_pct   = parseDouble(f[COL_PRECIP_PROB]);
    r.uv_index                 = parseDouble(f[COL_UV_INDEX]);
    r.solar_radiation_wm2      = parseDouble(f[COL_SOLAR_RADIATION]);
    r.weather_condition        = f[COL_WEATHER_CONDITION];
    r.weather_code             = parseInt(f[COL_WEATHER_CODE]);
    r.air_quality_index        = parseInt(f[COL_AQI]);
    r.pm2_5_ugm3               = parseDouble(f[COL_PM25]);
    r.pm10_ugm3                = parseDouble(f[COL_PM10]);
    r.ozone_ppb                = parseDouble(f[COL_OZONE]);
    r.no2_ppb                  = parseDouble(f[COL_NO2]);
    r.co_ppm                   = parseDouble(f[COL_CO]);
    r.sunrise_utc              = f[COL_SUNRISE];
    r.sunset_utc               = f[COL_SUNSET];
    r.daylight_hours           = parseDouble(f[COL_DAYLIGHT_HOURS]);
    r.soil_temp_c              = parseDouble(f[COL_SOIL_TEMP]);
    r.soil_moisture_pct        = parseDouble(f[COL_SOIL_MOISTURE]);
    r.evapotranspiration_mm    = parseDouble(f[COL_EVAPOTRANSPIRATION]);
    return r;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  WeatherModel — construction & CSV loading
// ═══════════════════════════════════════════════════════════════════════════════

WeatherModel::WeatherModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

void WeatherModel::clear()
{
    beginResetModel();
    m_records.clear();
    endResetModel();
}

bool WeatherModel::loadFromFile(const QString &filePath, QString *error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = file.errorString();
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    // Skip header line
    if (!in.atEnd()) in.readLine();

    QVector<WeatherRecord> loaded;
    loaded.reserve(1500);
    int lineNo = 1;

    while (!in.atEnd()) {
        ++lineNo;
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        const QStringList fields = line.split(',');
        if (fields.size() != COL_COUNT) {
            // Skip malformed rows; callers can check row count vs. expected 1500
            continue;
        }
        loaded.append(parseRow(fields));
    }

    beginResetModel();
    m_records = std::move(loaded);
    endResetModel();
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  QAbstractTableModel interface
// ═══════════════════════════════════════════════════════════════════════════════

int WeatherModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_records.size();
}

int WeatherModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : COL_COUNT;
}

QVariant WeatherModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        const QStringList headers = columnHeaders();
        if (section >= 0 && section < headers.size())
            return headers.at(section);
    }
    return {};
}

QVariant WeatherModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return {};
    const int row = index.row();
    const int col = index.column();
    if (row < 0 || row >= m_records.size()) return {};
    if (col < 0 || col >= COL_COUNT)        return {};

    const WeatherRecord &r = m_records.at(row);

    switch (role) {
    case Qt::DisplayRole:        return displayData(r, col);
    case Qt::UserRole:           return sortData(r, col);
    case Qt::BackgroundRole:     return backgroundFor(r, col);
    case Qt::ForegroundRole:     return foregroundFor(r, col);
    case Qt::TextAlignmentRole: {
        // Right-align numeric columns, left-align everything else
        static const QSet<int> numericCols = {
            COL_LATITUDE, COL_LONGITUDE, COL_ELEVATION_M,
            COL_TEMPERATURE_C, COL_TEMPERATURE_F, COL_FEELS_LIKE_C, COL_DEW_POINT_C,
            COL_HUMIDITY_PCT, COL_PRESSURE_STATION, COL_PRESSURE_SEA_LEVEL,
            COL_WIND_SPEED, COL_WIND_DIR_DEG, COL_WIND_GUST,
            COL_VISIBILITY, COL_CLOUD_COVER, COL_CLOUD_CEILING,
            COL_PRECIP_MM, COL_SNOW_DEPTH, COL_PRECIP_PROB,
            COL_UV_INDEX, COL_SOLAR_RADIATION, COL_WEATHER_CODE,
            COL_AQI, COL_PM25, COL_PM10, COL_OZONE, COL_NO2, COL_CO,
            COL_DAYLIGHT_HOURS, COL_SOIL_TEMP, COL_SOIL_MOISTURE, COL_EVAPOTRANSPIRATION
        };
        if (numericCols.contains(col))
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    default:
        return {};
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  data() role helpers
// ═══════════════════════════════════════════════════════════════════════════════

static QString fmtDouble(double v, int decimals = 1)
{
    return std::isnan(v) ? QStringLiteral("—") : QString::number(v, 'f', decimals);
}

static QString fmtInt(int v)
{
    return (v == -1) ? QStringLiteral("—") : QString::number(v);
}

QVariant WeatherModel::displayData(const WeatherRecord &r, int col) const
{
    switch (col) {
    // ── Identity / strings ──────────────────────────────────────────────────
    case COL_DATE:               return r.date;
    case COL_TIME_UTC:           return r.time_utc;
    case COL_STATION_ID:         return r.station_id;
    case COL_STATION_NAME:       return r.station_name;
    case COL_PRESSURE_TREND:     return r.pressure_trend;
    case COL_WIND_DIR_CARDINAL:  return r.wind_direction_cardinal;
    case COL_PRECIP_TYPE:        return r.precipitation_type;
    case COL_WEATHER_CONDITION:  return r.weather_condition;
    case COL_SUNRISE:            return r.sunrise_utc;
    case COL_SUNSET:             return r.sunset_utc;

    // ── Location ────────────────────────────────────────────────────────────
    case COL_LATITUDE:           return fmtDouble(r.latitude,   4);
    case COL_LONGITUDE:          return fmtDouble(r.longitude,  4);
    case COL_ELEVATION_M:        return fmtDouble(r.elevation_m, 0);

    // ── Temperature ─────────────────────────────────────────────────────────
    case COL_TEMPERATURE_C:      return fmtDouble(r.temperature_c,  1);
    case COL_TEMPERATURE_F:      return fmtDouble(r.temperature_f,  1);
    case COL_FEELS_LIKE_C:       return fmtDouble(r.feels_like_c,   1);
    case COL_DEW_POINT_C:        return fmtDouble(r.dew_point_c,    1);

    // ── Atmosphere ──────────────────────────────────────────────────────────
    case COL_HUMIDITY_PCT:       return fmtDouble(r.humidity_pct,         1);
    case COL_PRESSURE_STATION:   return fmtDouble(r.pressure_station_hpa, 1);
    case COL_PRESSURE_SEA_LEVEL: return fmtDouble(r.pressure_sea_level_hpa, 1);

    // ── Wind ────────────────────────────────────────────────────────────────
    case COL_WIND_SPEED:         return fmtDouble(r.wind_speed_kmh,     1);
    case COL_WIND_DIR_DEG:       return fmtDouble(r.wind_direction_deg, 0);
    case COL_WIND_GUST:          return fmtDouble(r.wind_gust_kmh,      1);

    // ── Visibility / cloud ──────────────────────────────────────────────────
    case COL_VISIBILITY:         return fmtDouble(r.visibility_km,   1);
    case COL_CLOUD_COVER:        return fmtDouble(r.cloud_cover_pct, 0);
    case COL_CLOUD_CEILING:      return fmtDouble(r.cloud_ceiling_m, 0);

    // ── Precipitation ───────────────────────────────────────────────────────
    case COL_PRECIP_MM:          return fmtDouble(r.precipitation_mm,      1);
    case COL_SNOW_DEPTH:         return fmtDouble(r.snow_depth_cm,          1);
    case COL_PRECIP_PROB:        return fmtDouble(r.precipitation_prob_pct, 0);

    // ── Solar / UV ──────────────────────────────────────────────────────────
    case COL_UV_INDEX:           return fmtDouble(r.uv_index,            1);
    case COL_SOLAR_RADIATION:    return fmtDouble(r.solar_radiation_wm2, 1);

    // ── Weather code ────────────────────────────────────────────────────────
    case COL_WEATHER_CODE:       return fmtInt(r.weather_code);

    // ── Air quality ─────────────────────────────────────────────────────────
    case COL_AQI:                return fmtInt(r.air_quality_index);
    case COL_PM25:               return fmtDouble(r.pm2_5_ugm3, 1);
    case COL_PM10:               return fmtDouble(r.pm10_ugm3,  1);
    case COL_OZONE:              return fmtDouble(r.ozone_ppb,  1);
    case COL_NO2:                return fmtDouble(r.no2_ppb,    1);
    case COL_CO:                 return fmtDouble(r.co_ppm,     2);

    // ── Daylight ────────────────────────────────────────────────────────────
    case COL_DAYLIGHT_HOURS:     return fmtDouble(r.daylight_hours, 1);

    // ── Soil ────────────────────────────────────────────────────────────────
    case COL_SOIL_TEMP:          return fmtDouble(r.soil_temp_c,         1);
    case COL_SOIL_MOISTURE:      return fmtDouble(r.soil_moisture_pct,   1);
    case COL_EVAPOTRANSPIRATION: return fmtDouble(r.evapotranspiration_mm, 2);

    default: return {};
    }
}

QVariant WeatherModel::sortData(const WeatherRecord &r, int col) const
{
    // For sort purposes: return raw double/int so the proxy's lessThan()
    // can compare numerically rather than lexicographically.
    switch (col) {
    case COL_LATITUDE:           return r.latitude;
    case COL_LONGITUDE:          return r.longitude;
    case COL_ELEVATION_M:        return r.elevation_m;
    case COL_TEMPERATURE_C:      return r.temperature_c;
    case COL_TEMPERATURE_F:      return r.temperature_f;
    case COL_FEELS_LIKE_C:       return r.feels_like_c;
    case COL_DEW_POINT_C:        return r.dew_point_c;
    case COL_HUMIDITY_PCT:       return r.humidity_pct;
    case COL_PRESSURE_STATION:   return r.pressure_station_hpa;
    case COL_PRESSURE_SEA_LEVEL: return r.pressure_sea_level_hpa;
    case COL_WIND_SPEED:         return r.wind_speed_kmh;
    case COL_WIND_DIR_DEG:       return r.wind_direction_deg;
    case COL_WIND_GUST:          return r.wind_gust_kmh;
    case COL_VISIBILITY:         return r.visibility_km;
    case COL_CLOUD_COVER:        return r.cloud_cover_pct;
    case COL_CLOUD_CEILING:      return r.cloud_ceiling_m;
    case COL_PRECIP_MM:          return r.precipitation_mm;
    case COL_SNOW_DEPTH:         return r.snow_depth_cm;
    case COL_PRECIP_PROB:        return r.precipitation_prob_pct;
    case COL_UV_INDEX:           return r.uv_index;
    case COL_SOLAR_RADIATION:    return r.solar_radiation_wm2;
    case COL_WEATHER_CODE:       return r.weather_code;
    case COL_AQI:                return r.air_quality_index;
    case COL_PM25:               return r.pm2_5_ugm3;
    case COL_PM10:               return r.pm10_ugm3;
    case COL_OZONE:              return r.ozone_ppb;
    case COL_NO2:                return r.no2_ppb;
    case COL_CO:                 return r.co_ppm;
    case COL_DAYLIGHT_HOURS:     return r.daylight_hours;
    case COL_SOIL_TEMP:          return r.soil_temp_c;
    case COL_SOIL_MOISTURE:      return r.soil_moisture_pct;
    case COL_EVAPOTRANSPIRATION: return r.evapotranspiration_mm;
    default:                     return displayData(r, col); // string cols sort lexicographically
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Colour coding
// ═══════════════════════════════════════════════════════════════════════════════

// Temperature: interpolate cold (blue) → mild (white) → hot (red)
// Range mapped: -30 °C → 0 °C → 40 °C
QColor WeatherModel::temperatureColor(double tempC)
{
    if (std::isnan(tempC)) return {};

    // Clamp to [-30, 40]
    const double t = qBound(-30.0, tempC, 40.0);

    int r, g, b;
    if (t < 0.0) {
        // cold: blue (0,100,200) → white (240,240,240) over [-30, 0]
        const double ratio = (t + 30.0) / 30.0;  // 0 at -30, 1 at 0
        r = static_cast<int>(  0 + ratio * 240);
        g = static_cast<int>(100 + ratio * 140);
        b = static_cast<int>(200 + ratio *  40);
    } else {
        // warm: white (240,240,240) → red (220,60,60) over [0, 40]
        const double ratio = t / 40.0;            // 0 at 0, 1 at 40
        r = static_cast<int>(240 - ratio *  20);
        g = static_cast<int>(240 - ratio * 180);
        b = static_cast<int>(240 - ratio * 180);
    }
    return QColor(r, g, b);
}

// AQI colours: US EPA standard bands
// 0-50 Good (green), 51-100 Moderate (yellow), 101-150 Unhealthy for Sensitive (orange),
// 151-200 Unhealthy (red), 201-300 Very Unhealthy (purple), 301+ Hazardous (maroon)
QColor WeatherModel::aqiColor(int aqi)
{
    if (aqi < 0) return {};
    if (aqi <=  50) return QColor(  0, 180,  96);   // green
    if (aqi <= 100) return QColor(255, 222,  51);   // yellow
    if (aqi <= 150) return QColor(255, 126,   0);   // orange
    if (aqi <= 200) return QColor(255,  60,  60);   // red
    if (aqi <= 300) return QColor(143,  63, 151);   // purple
    return              QColor(126,   0,  35);      // maroon
}

// Precipitation type: tinted backgrounds
QColor WeatherModel::precipTypeColor(const QString &type)
{
    if (type.isEmpty() || type == "none") return {};
    if (type == "rain")        return QColor(173, 216, 230);  // light blue
    if (type == "snow")        return QColor(220, 230, 255);  // pale lavender
    if (type == "thunderstorm")return QColor(255, 200, 100);  // amber
    if (type == "hail")        return QColor(255, 180, 180);  // soft red
    if (type == "freezing rain")return QColor(200, 220, 255); // icy blue
    return QColor(220, 220, 220);  // grey for any other type
}

QVariant WeatherModel::backgroundFor(const WeatherRecord &r, int col) const
{
    switch (col) {
    case COL_TEMPERATURE_C:
    case COL_TEMPERATURE_F:
    case COL_FEELS_LIKE_C:
    case COL_DEW_POINT_C:
    case COL_SOIL_TEMP: {
        // Pick the °C value regardless of which column we're colouring
        double tempC = (col == COL_TEMPERATURE_F)
            ? (r.temperature_f - 32.0) * 5.0 / 9.0
            : (col == COL_TEMPERATURE_C ? r.temperature_c
               : col == COL_FEELS_LIKE_C ? r.feels_like_c
               : col == COL_DEW_POINT_C ? r.dew_point_c
               : r.soil_temp_c);
        QColor c = temperatureColor(tempC);
        return c.isValid() ? QBrush(c) : QVariant{};
    }
    case COL_AQI:
    case COL_PM25:
    case COL_PM10:
    case COL_OZONE:
    case COL_NO2:
    case COL_CO: {
        QColor c = aqiColor(r.air_quality_index);
        return c.isValid() ? QBrush(c) : QVariant{};
    }
    case COL_PRECIP_TYPE: {
        QColor c = precipTypeColor(r.precipitation_type);
        return c.isValid() ? QBrush(c) : QVariant{};
    }
    default:
        return {};
    }
}

QVariant WeatherModel::foregroundFor(const WeatherRecord &r, int col) const
{
    // Use white text on the two darkest AQI bands (unhealthy red, purple, maroon)
    if (col == COL_AQI || col == COL_PM25 || col == COL_PM10 ||
        col == COL_OZONE || col == COL_NO2 || col == COL_CO)
    {
        if (r.air_quality_index >= 150)
            return QBrush(Qt::white);
    }
    return {};
}

// ═══════════════════════════════════════════════════════════════════════════════
//  WeatherProxyModel
// ═══════════════════════════════════════════════════════════════════════════════

WeatherProxyModel::WeatherProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortRole(Qt::UserRole);   // sort by raw values, not display strings
}

void WeatherProxyModel::setFilterText(const QString &text)
{
    m_filterText = text.trimmed();
    invalidateFilter();
}

bool WeatherProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_filterText.isEmpty()) return true;

    const QAbstractItemModel *src = sourceModel();
    const int cols = src->columnCount(sourceParent);
    for (int col = 0; col < cols; ++col) {
        const QModelIndex idx = src->index(sourceRow, col, sourceParent);
        if (src->data(idx, Qt::DisplayRole).toString().contains(m_filterText, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

bool WeatherProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const QVariant lv = sourceModel()->data(left,  Qt::UserRole);
    const QVariant rv = sourceModel()->data(right, Qt::UserRole);

    // Double comparison: NaN sorts to the end
    if (lv.userType() == QMetaType::Double && rv.userType() == QMetaType::Double) {
        const double ld = lv.toDouble();
        const double rd = rv.toDouble();
        if (std::isnan(ld)) return false;
        if (std::isnan(rd)) return true;
        return ld < rd;
    }

    // Int comparison: -1 (missing) sorts to the end
    if (lv.userType() == QMetaType::Int && rv.userType() == QMetaType::Int) {
        const int li = lv.toInt();
        const int ri = rv.toInt();
        if (li == -1) return false;
        if (ri == -1) return true;
        return li < ri;
    }

    // Fall back to string compare (handles all string columns, dates, times)
    return lv.toString().compare(rv.toString(), Qt::CaseInsensitive) < 0;
}
