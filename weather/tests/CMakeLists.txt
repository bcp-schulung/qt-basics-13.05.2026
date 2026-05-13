cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Widgets Sql)

# Helper macro: create a test executable and register it with CTest.
macro(add_qt_test TARGET)
    add_executable(${TARGET} ${ARGN})
    target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
    add_test(NAME ${TARGET} COMMAND ${TARGET})
endmacro()

# ─── tst_WeatherRecord ───────────────────────────────────────────────────────
# Tests the WeatherRecord struct defaults and field assignment.
# No extra source files needed — WeatherRecord is header-only.
add_qt_test(tst_WeatherRecord
    tst_WeatherRecord.cpp
)
target_link_libraries(tst_WeatherRecord PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)

# ─── tst_WeatherModel ────────────────────────────────────────────────────────
# Tests WeatherModel (rowCount, columnCount, data(), setRecords(), loadFromFile).
add_qt_test(tst_WeatherModel
    tst_WeatherModel.cpp
    ../WeatherModel.cpp
)
target_link_libraries(tst_WeatherModel PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)

# ─── tst_WeatherDatabase ─────────────────────────────────────────────────────
# Tests WeatherDatabaseWorker signals using QSignalSpy + real SQLite.
add_qt_test(tst_WeatherDatabase
    tst_WeatherDatabase.cpp
    ../WeatherDatabase.cpp
)
target_link_libraries(tst_WeatherDatabase PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)
