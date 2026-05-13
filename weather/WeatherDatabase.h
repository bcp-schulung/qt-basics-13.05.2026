#pragma once

#include <QObject>
#include <QVector>
#include "WeatherRecord.h"

// Required so QVector<WeatherRecord> can cross thread boundaries via queued signals.
Q_DECLARE_METATYPE(QVector<WeatherRecord>)

// ─── WeatherDatabaseWorker ─────────────────────────────────────────────────────
// Runs on a dedicated QThread. All database I/O (open, schema creation, load,
// save) happens in that thread — never on the UI thread.
//
// Lifecycle:
//   1. Create worker (no parent) and a QThread.
//   2. worker->moveToThread(thread).
//   3. Connect QThread::started → worker::initAndLoad.
//   4. Connect QThread::finished → worker::deleteLater.
//   5. thread->start() — triggers initAndLoad() automatically.
//   6. After a CSV load, call save() via QMetaObject::invokeMethod or a
//      connected signal (both are queued and thread-safe).
//
// The SQLite connection is opened in initAndLoad() inside the worker thread so
// that Qt's SQL driver rule ("connection used from the thread that created it")
// is satisfied.
class WeatherDatabaseWorker : public QObject
{
    Q_OBJECT

public:
    explicit WeatherDatabaseWorker(const QString &dbPath, QObject *parent = nullptr);
    ~WeatherDatabaseWorker() override;

public slots:
    // Opens the DB, creates the schema if needed, then loads all persisted rows.
    // Connected to QThread::started — do not call manually.
    void initAndLoad();

    // Replaces all rows in the DB with the supplied records inside a single
    // transaction. Safe to call from the main thread via a queued connection.
    void save(const QVector<WeatherRecord> &records);

signals:
    // Emitted after initAndLoad(); records is empty when the DB is brand-new.
    void loadCompleted(QVector<WeatherRecord> records);

    // Emitted after a successful save(); count == records.size().
    void saveCompleted(int count);

    // Emitted on any SQL error; the worker remains usable after the error.
    void errorOccurred(QString message);

private:
    QString m_dbPath;
    QString m_connectionName;   // unique per instance to avoid driver conflicts

    bool openDatabase();
    bool createSchema();
    QVector<WeatherRecord> loadAll();
};
