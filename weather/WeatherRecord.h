#pragma once

#include <QString>
#include <QStringList>
#include <QList>

// ─── Flat data struct for one weather observation ────────────────────────────────
// All numeric fields use qQNaN() as a "missing" sentinel.
// Only cloud_ceiling_m is nullable in this dataset; all others are fully populated.
struct WeatherRecord {
    // Identity
    QString date;
    QString time_utc;
    QString station_id;
    QString station_name;

    // Location
    double  latitude               = qQNaN();
    double  longitude              = qQNaN();
    double  elevation_m            = qQNaN();

    // Temperature
    double  temperature_c          = qQNaN();
    double  temperature_f          = qQNaN();
    double  feels_like_c           = qQNaN();
    double  dew_point_c            = qQNaN();

    // Atmosphere
    double  humidity_pct           = qQNaN();
    double  pressure_station_hpa   = qQNaN();
    double  pressure_sea_level_hpa = qQNaN();
    QString pressure_trend;

    // Wind
    double  wind_speed_kmh         = qQNaN();
    double  wind_direction_deg     = qQNaN();
    QString wind_direction_cardinal;
    double  wind_gust_kmh          = qQNaN();

    // Visibility & clouds
    double  visibility_km          = qQNaN();
    double  cloud_cover_pct        = qQNaN();
    double  cloud_ceiling_m        = qQNaN(); // NaN = missing (~574 rows)

    // Precipitation
    QString precipitation_type;
    double  precipitation_mm       = qQNaN();
    double  snow_depth_cm          = qQNaN();
    double  precipitation_prob_pct = qQNaN();

    // Solar / UV
    double  uv_index               = qQNaN();
    double  solar_radiation_wm2    = qQNaN();

    // Weather
    QString weather_condition;
    int     weather_code           = -1;  // -1 = missing

    // Air quality
    int     air_quality_index      = -1;  // -1 = missing
    double  pm2_5_ugm3             = qQNaN();
    double  pm10_ugm3              = qQNaN();
    double  ozone_ppb              = qQNaN();
    double  no2_ppb                = qQNaN();
    double  co_ppm                 = qQNaN();

    // Daylight
    QString sunrise_utc;
    QString sunset_utc;
    double  daylight_hours         = qQNaN();

    // Soil
    double  soil_temp_c            = qQNaN();
    double  soil_moisture_pct      = qQNaN();
    double  evapotranspiration_mm  = qQNaN();
};

// ─── Column index enum (matches CSV field order exactly) ────────────────────────
enum Column : int {
    COL_DATE = 0,
    COL_TIME_UTC,
    COL_STATION_ID,
    COL_STATION_NAME,
    COL_LATITUDE,
    COL_LONGITUDE,
    COL_ELEVATION_M,
    COL_TEMPERATURE_C,
    COL_TEMPERATURE_F,
    COL_FEELS_LIKE_C,
    COL_DEW_POINT_C,
    COL_HUMIDITY_PCT,
    COL_PRESSURE_STATION,
    COL_PRESSURE_SEA_LEVEL,
    COL_PRESSURE_TREND,
    COL_WIND_SPEED,
    COL_WIND_DIR_DEG,
    COL_WIND_DIR_CARDINAL,
    COL_WIND_GUST,
    COL_VISIBILITY,
    COL_CLOUD_COVER,
    COL_CLOUD_CEILING,
    COL_PRECIP_TYPE,
    COL_PRECIP_MM,
    COL_SNOW_DEPTH,
    COL_PRECIP_PROB,
    COL_UV_INDEX,
    COL_SOLAR_RADIATION,
    COL_WEATHER_CONDITION,
    COL_WEATHER_CODE,
    COL_AQI,
    COL_PM25,
    COL_PM10,
    COL_OZONE,
    COL_NO2,
    COL_CO,
    COL_SUNRISE,
    COL_SUNSET,
    COL_DAYLIGHT_HOURS,
    COL_SOIL_TEMP,
    COL_SOIL_MOISTURE,
    COL_EVAPOTRANSPIRATION,
    COL_COUNT   // always last — used as columnCount()
};

// ─── Column display headers ──────────────────────────────────────────────────────
inline QStringList columnHeaders()
{
    return {
        "Date",                 // 0
        "Time (UTC)",           // 1
        "Station ID",           // 2
        "Station Name",         // 3
        "Latitude",             // 4
        "Longitude",            // 5
        "Elevation (m)",        // 6
        "Temp (°C)",            // 7
        "Temp (°F)",            // 8
        "Feels Like (°C)",      // 9
        "Dew Point (°C)",       // 10
        "Humidity (%)",         // 11
        "Pressure Stn (hPa)",   // 12
        "Pressure SL (hPa)",    // 13
        "Pressure Trend",       // 14
        "Wind Speed (km/h)",    // 15
        "Wind Dir (°)",         // 16
        "Wind Dir",             // 17
        "Wind Gust (km/h)",     // 18
        "Visibility (km)",      // 19
        "Cloud Cover (%)",      // 20
        "Cloud Ceiling (m)",    // 21
        "Precip. Type",         // 22
        "Precip. (mm)",         // 23
        "Snow Depth (cm)",      // 24
        "Precip. Prob. (%)",    // 25
        "UV Index",             // 26
        "Solar Rad. (W/m²)",    // 27
        "Weather Condition",    // 28
        "Weather Code",         // 29
        "AQI",                  // 30
        "PM2.5 (µg/m³)",        // 31
        "PM10 (µg/m³)",         // 32
        "Ozone (ppb)",          // 33
        "NO₂ (ppb)",            // 34
        "CO (ppm)",             // 35
        "Sunrise (UTC)",        // 36
        "Sunset (UTC)",         // 37
        "Daylight (h)",         // 38
        "Soil Temp (°C)",       // 39
        "Soil Moisture (%)",    // 40
        "Evapotrans. (mm)"      // 41
    };
}

// ─── Column group definitions (for visibility toggle menu) ──────────────────────
struct ColumnGroup {
    QString    name;
    QList<int> columns;
};

inline QList<ColumnGroup> columnGroups()
{
    return {
        { "Identity",      { COL_DATE, COL_TIME_UTC, COL_STATION_ID, COL_STATION_NAME } },
        { "Location",      { COL_LATITUDE, COL_LONGITUDE, COL_ELEVATION_M } },
        { "Temperature",   { COL_TEMPERATURE_C, COL_TEMPERATURE_F, COL_FEELS_LIKE_C, COL_DEW_POINT_C } },
        { "Atmosphere",    { COL_HUMIDITY_PCT, COL_PRESSURE_STATION, COL_PRESSURE_SEA_LEVEL, COL_PRESSURE_TREND } },
        { "Wind",          { COL_WIND_SPEED, COL_WIND_DIR_DEG, COL_WIND_DIR_CARDINAL, COL_WIND_GUST } },
        { "Visibility",    { COL_VISIBILITY, COL_CLOUD_COVER, COL_CLOUD_CEILING } },
        { "Precipitation", { COL_PRECIP_TYPE, COL_PRECIP_MM, COL_SNOW_DEPTH, COL_PRECIP_PROB } },
        { "Solar / UV",    { COL_UV_INDEX, COL_SOLAR_RADIATION } },
        { "Weather",       { COL_WEATHER_CONDITION, COL_WEATHER_CODE } },
        { "Air Quality",   { COL_AQI, COL_PM25, COL_PM10, COL_OZONE, COL_NO2, COL_CO } },
        { "Daylight",      { COL_SUNRISE, COL_SUNSET, COL_DAYLIGHT_HOURS } },
        { "Soil",          { COL_SOIL_TEMP, COL_SOIL_MOISTURE, COL_EVAPOTRANSPIRATION } },
    };
}
