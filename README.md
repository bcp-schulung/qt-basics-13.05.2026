# Weather Dataset — Educational Reference

`weather_data.csv` is a synthetic but physically plausible weather observation dataset designed for learning data analysis, visualisation, and statistics. It contains **1,500 rows** and **42 columns** spread across 30 US weather stations over two calendar years (2023–2024).

---

## File

| File | Rows | Columns | Size |
|------|------|---------|------|
| `weather_data.csv` | 1,500 | 42 | ~354 KB |

---

## Stations

| ID | Name | City | Lat | Lon | Elev (m) |
|----|------|------|-----|-----|-----------|
| NYC001 | New York – Central Park | New York, NY | 40.78 | -73.97 | 39 |
| LAX001 | Los Angeles – Downtown | Los Angeles, CA | 34.05 | -118.24 | 71 |
| ORD001 | Chicago – O'Hare Area | Chicago, IL | 41.97 | -87.91 | 201 |
| IAH001 | Houston – Hobby | Houston, TX | 29.65 | -95.28 | 15 |
| MIA001 | Miami – International | Miami, FL | 25.80 | -80.29 | 3 |
| DEN001 | Denver – Stapleton | Denver, CO | 39.74 | -104.99 | 1,609 |
| SEA001 | Seattle – Sea-Tac | Seattle, WA | 47.45 | -122.31 | 137 |
| PHX001 | Phoenix – Sky Harbor | Phoenix, AZ | 33.44 | -112.01 | 337 |
| BOS001 | Boston – Logan | Boston, MA | 42.37 | -71.01 | 9 |
| MSP001 | Minneapolis – St Paul | Minneapolis, MN | 44.88 | -93.22 | 255 |
| ATL001 | Atlanta – Hartsfield | Atlanta, GA | 33.64 | -84.43 | 313 |
| DFW001 | Dallas – Fort Worth | Dallas, TX | 32.90 | -97.04 | 182 |
| SFO001 | San Francisco – Intl | San Francisco, CA | 37.62 | -122.38 | 4 |
| LAS001 | Las Vegas – McCarran | Las Vegas, NV | 36.08 | -115.15 | 664 |
| PDX001 | Portland – Intl | Portland, OR | 45.59 | -122.60 | 9 |
| SLC001 | Salt Lake City – Intl | Salt Lake City, UT | 40.79 | -111.98 | 1,288 |
| MCI001 | Kansas City – Intl | Kansas City, MO | 39.30 | -94.71 | 313 |
| STL001 | St Louis – Lambert | St Louis, MO | 38.75 | -90.37 | 172 |
| CLE001 | Cleveland – Hopkins | Cleveland, OH | 41.41 | -81.85 | 245 |
| DTW001 | Detroit – Metro Wayne | Detroit, MI | 42.22 | -83.36 | 195 |
| PHL001 | Philadelphia – Intl | Philadelphia, PA | 39.87 | -75.24 | 9 |
| BWI001 | Baltimore – Washington | Baltimore, MD | 39.18 | -76.67 | 46 |
| RDU001 | Raleigh-Durham – Intl | Raleigh, NC | 35.88 | -78.79 | 132 |
| TPA001 | Tampa – Intl | Tampa, FL | 27.98 | -82.53 | 9 |
| MSY001 | New Orleans – Moisant | New Orleans, LA | 29.99 | -90.26 | 3 |
| ABQ001 | Albuquerque – Intl Sunport | Albuquerque, NM | 35.04 | -106.61 | 1,619 |
| ANC001 | Anchorage – Intl | Anchorage, AK | 61.17 | -149.99 | 46 |
| HNL001 | Honolulu – Intl | Honolulu, HI | 21.33 | -157.93 | 4 |
| MKE001 | Milwaukee – Mitchell | Milwaukee, WI | 42.95 | -87.90 | 211 |
| IND001 | Indianapolis – Intl | Indianapolis, IN | 39.72 | -86.29 | 248 |

Each station has 50 observations spread across 2023–2024, recorded at random hours throughout the day.

---

## Column Reference

### Identification & Location

| Column | Type | Description |
|--------|------|-------------|
| `date` | date (YYYY-MM-DD) | Observation date |
| `time_utc` | time (HH:MM) | Observation time in UTC |
| `station_id` | string | Unique station code |
| `station_name` | string | Human-readable station name |
| `latitude` | float (°) | Station latitude (WGS-84) |
| `longitude` | float (°) | Station longitude (WGS-84) |
| `elevation_m` | integer (m) | Station elevation above sea level |

### Temperature

| Column | Type | Description |
|--------|------|-------------|
| `temperature_c` | float (°C) | Dry-bulb air temperature |
| `temperature_f` | float (°F) | Same temperature in Fahrenheit |
| `feels_like_c` | float (°C) | Apparent temperature (heat index above 27 °C; wind-chill at or below 10 °C) |
| `dew_point_c` | float (°C) | Dew point temperature |

### Humidity & Pressure

| Column | Type | Description |
|--------|------|-------------|
| `humidity_pct` | integer (%) | Relative humidity |
| `pressure_station_hpa` | float (hPa) | Barometric pressure at station elevation |
| `pressure_sea_level_hpa` | float (hPa) | Barometric pressure reduced to sea level (MSLP) |
| `pressure_trend` | string | Short-term trend: `rising`, `falling`, `steady`, `rising rapidly`, `falling rapidly` |

### Wind

| Column | Type | Description |
|--------|------|-------------|
| `wind_speed_kmh` | float (km/h) | 10-minute mean wind speed |
| `wind_direction_deg` | integer (°) | Wind direction in degrees from north (0–359) |
| `wind_direction_cardinal` | string | 16-point cardinal direction (N, NNE, NE, …) |
| `wind_gust_kmh` | float (km/h) | Peak 3-second gust speed |

### Visibility & Cloud

| Column | Type | Description |
|--------|------|-------------|
| `visibility_km` | float (km) | Prevailing visibility |
| `cloud_cover_pct` | integer (%) | Total cloud cover (0 = clear sky, 100 = overcast) |
| `cloud_ceiling_m` | integer (m) | Height of lowest cloud layer; empty when cloud cover ≤ 30 % |

### Precipitation

| Column | Type | Description |
|--------|------|-------------|
| `precipitation_type` | string | Type: `none`, `rain`, `snow`, `sleet`, `hail`, `thunderstorm` |
| `precipitation_mm` | float (mm) | Liquid-equivalent precipitation depth (0 for snow-only events) |
| `snow_depth_cm` | float (cm) | Snow accumulation (0 for non-snow events) |
| `precipitation_prob_pct` | integer (%) | Probability of precipitation at observation time |

### Solar & UV

| Column | Type | Description |
|--------|------|-------------|
| `uv_index` | float | UV index (0 at night; scaled by cloud cover) |
| `solar_radiation_wm2` | float (W/m²) | Incoming shortwave solar radiation (0 at night) |
| `sunrise_utc` | time (HH:MM) | Approximate sunrise time in UTC |
| `sunset_utc` | time (HH:MM) | Approximate sunset time in UTC |
| `daylight_hours` | integer (h) | Total hours of daylight for that day |

### Soil & Evapotranspiration

| Column | Type | Description |
|--------|------|-------------|
| `soil_temp_c` | float (°C) | Estimated near-surface soil temperature |
| `soil_moisture_pct` | float (%) | Volumetric soil moisture content |
| `evapotranspiration_mm` | float (mm) | Estimated daily evapotranspiration |

### Weather Condition

| Column | Type | Description |
|--------|------|-------------|
| `weather_condition` | string | Plain-text description (e.g. "Light Rain", "Thunderstorm") |
| `weather_code` | integer | Numeric code derived from precipitation type + condition index |

### Air Quality

| Column | Type | Description |
|--------|------|-------------|
| `air_quality_index` | integer (AQI) | US AQI scale (0–500; higher = worse) |
| `pm2_5_ugm3` | float (µg/m³) | Fine particulate matter (PM2.5) |
| `pm10_ugm3` | float (µg/m³) | Coarse particulate matter (PM10) |
| `ozone_ppb` | float (ppb) | Ground-level ozone concentration |
| `no2_ppb` | float (ppb) | Nitrogen dioxide concentration |
| `co_ppm` | float (ppm) | Carbon monoxide concentration |

---

## Data Generation Notes

The dataset is **synthetic** — it was generated with a Python script using realistic physical models:

- **Temperature** follows a latitude-based baseline, a seasonal cosine curve, an elevation lapse rate (6.5 °C / 1 000 m), and a diurnal sine swing, with Gaussian noise added.
- **Feels-like** uses the NWS Heat Index formula above 27 °C and the Environment Canada wind-chill formula at or below 10 °C.
- **Precipitation type** is weighted by temperature (snow when T < 0 °C, mixed precipitation near 0–2 °C).
- **Solar radiation & UV** are sinusoidally modelled for daylight hours and attenuated by cloud cover.
- **Air quality** values are randomly drawn within plausible US urban ranges.

Because values are generated algorithmically, correlations between related fields (e.g. temperature and feels-like, precipitation type and temperature) are physically consistent and suitable for exploratory analysis.

---

## Suggested Exercises

1. **Descriptive statistics** — compute mean, median, and standard deviation of `temperature_c` per station.
2. **Correlation analysis** — explore the relationship between `humidity_pct` and `dew_point_c`.
3. **Seasonal trends** — plot monthly average temperature for each station.
4. **Filtering & grouping** — how many thunderstorm events occurred? Which station had the most?
5. **Unit conversion** — verify that `temperature_f = temperature_c × 9/5 + 32`.
6. **Missing data** — `cloud_ceiling_m` is intentionally empty for clear-sky observations; practice handling `NaN` values.
7. **Wind rose** — visualise wind direction distribution using `wind_direction_cardinal`.
8. **Air quality** — identify days with AQI > 150 and investigate associated weather conditions.
9. **Elevation effect** — compare `pressure_station_hpa` vs `pressure_sea_level_hpa` across stations with different elevations.
10. **Time-series** — build a line chart of temperature over time for a single station.

---

## Quick Start (Python / pandas)

```python
import pandas as pd

df = pd.read_csv("weather_data.csv", parse_dates=["date"])
print(df.shape)          # (1500, 42)
print(df.dtypes)
print(df.describe())

# Mean temperature per station
print(df.groupby("station_name")["temperature_c"].mean().sort_values())
```

---

*Generated for educational use. Values are plausible but not real observations.*
