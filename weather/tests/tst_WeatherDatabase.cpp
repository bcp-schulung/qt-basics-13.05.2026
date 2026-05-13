#include <QTest>
#include <QSignalSpy>
#include <QThread>
#include <QTemporaryDir>

#include "WeatherDatabase.h"

// ─── RAII wrapper that runs a WeatherDatabaseWorker on a QThread ──────────────
//
// Usage:
//   WorkerScope scope("/path/to/test.sqlite");
//   QSignalSpy   spy(scope.worker(), &WeatherDatabaseWorker::loadCompleted);
//   scope.start();
//   QVERIFY(spy.wait(3000));
//
struct WorkerScope
{
    explicit WorkerScope(const QString &dbPath)
        : m_thread(new QThread)
        , m_worker(new WeatherDatabaseWorker(dbPath))
    {
        m_worker->moveToThread(m_thread);
        QObject::connect(m_thread, &QThread::started,
                         m_worker, &WeatherDatabaseWorker::initAndLoad);
        QObject::connect(m_thread, &QThread::finished,
                         m_worker, &QObject::deleteLater);
    }

    ~WorkerScope()
    {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        // m_worker is deleted by deleteLater via QThread::finished
    }

    WeatherDatabaseWorker *worker() const { return m_worker; }
    void start() { m_thread->start(); }

private:
    QThread               *m_thread;
    WeatherDatabaseWorker *m_worker;
};

// ─── Helper ──────────────────────────────────────────────────────────────────

static WeatherRecord makeRecord(const QString &date)
{
    WeatherRecord r;
    r.date          = date;
    r.time_utc      = QStringLiteral("00:00");
    r.station_id    = QStringLiteral("TST");
    r.station_name  = QStringLiteral("TestStation");
    r.temperature_c = 10.0;
    return r;
}

// ─── Test class ──────────────────────────────────────────────────────────────

class TestWeatherDatabase : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void freshDb_emitsEmptyVector();
    void save_emitsSaveCompletedWithCount();
    void saveAndReload_persistsRecords();
    void clearCache_removesAllRows();
};

void TestWeatherDatabase::initTestCase()
{
    // Register the custom type so QSignalSpy can capture it via queued connections
    // (worker thread → test thread).
    qRegisterMetaType<QVector<WeatherRecord>>("QVector<WeatherRecord>");
}

// A brand-new database must emit loadCompleted with an empty vector.
void TestWeatherDatabase::freshDb_emitsEmptyVector()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    WorkerScope scope(dir.filePath(QStringLiteral("fresh.sqlite")));
    QSignalSpy spy(scope.worker(), &WeatherDatabaseWorker::loadCompleted);
    scope.start();

    QVERIFY(spy.wait(3000));
    QCOMPARE(spy.count(), 1);
    const auto records = spy.at(0).at(0).value<QVector<WeatherRecord>>();
    QVERIFY(records.isEmpty());
}

// save() must emit saveCompleted(n) where n == records.size().
void TestWeatherDatabase::save_emitsSaveCompletedWithCount()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    WorkerScope scope(dir.filePath(QStringLiteral("save.sqlite")));
    QSignalSpy loadSpy(scope.worker(), &WeatherDatabaseWorker::loadCompleted);
    QSignalSpy saveSpy(scope.worker(), &WeatherDatabaseWorker::saveCompleted);
    scope.start();

    QVERIFY(loadSpy.wait(3000));

    QVector<WeatherRecord> records;
    records << makeRecord("2024-01-01")
            << makeRecord("2024-01-02")
            << makeRecord("2024-01-03");

    QMetaObject::invokeMethod(scope.worker(), "save",
                              Qt::QueuedConnection,
                              Q_ARG(QVector<WeatherRecord>, records));

    QVERIFY(saveSpy.wait(3000));
    QCOMPARE(saveSpy.count(), 1);
    QCOMPARE(saveSpy.at(0).at(0).toInt(), 3);
}

// Records saved to the DB must survive a worker restart (i.e. they are persisted).
void TestWeatherDatabase::saveAndReload_persistsRecords()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString dbPath = dir.filePath(QStringLiteral("persist.sqlite"));

    // First run: save two records.
    {
        WorkerScope scope(dbPath);
        QSignalSpy loadSpy(scope.worker(), &WeatherDatabaseWorker::loadCompleted);
        QSignalSpy saveSpy(scope.worker(), &WeatherDatabaseWorker::saveCompleted);
        scope.start();

        QVERIFY(loadSpy.wait(3000));

        QVector<WeatherRecord> records;
        records << makeRecord("2024-02-01")
                << makeRecord("2024-02-02");

        QMetaObject::invokeMethod(scope.worker(), "save",
                                  Qt::QueuedConnection,
                                  Q_ARG(QVector<WeatherRecord>, records));
        QVERIFY(saveSpy.wait(3000));
    }
    // WorkerScope destructor calls thread.quit() / thread.wait() — clean teardown.

    // Second run: reload and verify the two records come back.
    {
        WorkerScope scope2(dbPath);
        QSignalSpy loadSpy2(scope2.worker(), &WeatherDatabaseWorker::loadCompleted);
        scope2.start();

        QVERIFY(loadSpy2.wait(3000));
        QCOMPARE(loadSpy2.count(), 1);

        const auto loaded = loadSpy2.at(0).at(0).value<QVector<WeatherRecord>>();
        QCOMPARE(loaded.size(), 2);
        QCOMPARE(loaded.at(0).date, QStringLiteral("2024-02-01"));
        QCOMPARE(loaded.at(1).date, QStringLiteral("2024-02-02"));
    }
}

// clearCache() must delete all rows and emit cacheCleared().
void TestWeatherDatabase::clearCache_removesAllRows()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    WorkerScope scope(dir.filePath(QStringLiteral("clear.sqlite")));
    QSignalSpy loadSpy (scope.worker(), &WeatherDatabaseWorker::loadCompleted);
    QSignalSpy saveSpy (scope.worker(), &WeatherDatabaseWorker::saveCompleted);
    QSignalSpy clearSpy(scope.worker(), &WeatherDatabaseWorker::cacheCleared);
    scope.start();

    QVERIFY(loadSpy.wait(3000));

    // Save one record first.
    QVector<WeatherRecord> records;
    records << makeRecord("2024-03-01");
    QMetaObject::invokeMethod(scope.worker(), "save",
                              Qt::QueuedConnection,
                              Q_ARG(QVector<WeatherRecord>, records));
    QVERIFY(saveSpy.wait(3000));

    // Now clear.
    QMetaObject::invokeMethod(scope.worker(), "clearCache", Qt::QueuedConnection);
    QVERIFY(clearSpy.wait(3000));
    QCOMPARE(clearSpy.count(), 1);
}

QTEST_MAIN(TestWeatherDatabase)
#include "tst_WeatherDatabase.moc"
