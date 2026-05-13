#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "WeatherModel.h"
#include "TemperatureAreaChart.h"
#include "TemperatureBandWidget.h"
#include "WeatherDatabase.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QStatusBar>
#include <cmath>
#include <limits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_model = new WeatherModel(this);
    m_proxy = new WeatherProxyModel(this);
    m_proxy->setSourceModel(m_model);
    ui->tableView->setModel(m_proxy);
    ui->tableView->setSortingEnabled(true);

    ui->mostInterestingElements->setModel(m_proxy);
    ui->mostInterestingElements->setSortingEnabled(true);

    // ── Tab 3: Temperature area chart ─────────────────────────────────────────
    m_tempChart = new TemperatureAreaChart(ui->tab_2);
    auto *chartLayout = new QVBoxLayout(ui->tab_2);
    chartLayout->setContentsMargins(4, 4, 4, 4);
    chartLayout->addWidget(m_tempChart);

    // ── SQLite worker on a dedicated background thread ─────────────────────────
    const QString dbDir  = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dbPath = dbDir + QStringLiteral("/weather_cache.db");

    m_dbWorker = new WeatherDatabaseWorker(dbPath);   // no parent — moved to thread
    m_dbThread = new QThread(this);
    m_dbWorker->moveToThread(m_dbThread);

    // Wire worker signals → main-thread slots (automatically queued).
    connect(m_dbWorker, &WeatherDatabaseWorker::loadCompleted,
            this,        &MainWindow::onDbLoadCompleted);
    connect(m_dbWorker, &WeatherDatabaseWorker::saveCompleted,
            this,        &MainWindow::onDbSaveCompleted);
    connect(m_dbWorker, &WeatherDatabaseWorker::errorOccurred,
            this,        &MainWindow::onDbError);

    // Expose a signal so the main thread can safely enqueue a save request.
    connect(this,        &MainWindow::requestDbSave,
            m_dbWorker,  &WeatherDatabaseWorker::save);

    // Thread lifetime management.
    connect(m_dbThread, &QThread::started,  m_dbWorker, &WeatherDatabaseWorker::initAndLoad);
    connect(m_dbThread, &QThread::finished, m_dbWorker, &QObject::deleteLater);

    m_dbThread->start();
    statusBar()->showMessage(tr("Loading cached data…"));
}

MainWindow::~MainWindow()
{
    m_dbThread->quit();
    m_dbThread->wait();
    delete ui;
}

// ─── Private helpers ──────────────────────────────────────────────────────────

void MainWindow::populateUi(const QVector<WeatherRecord> &records)
{
    ui->totalEntries->display(records.size());

    double highest = -std::numeric_limits<double>::infinity();
    double lowest  =  std::numeric_limits<double>::infinity();
    for (const WeatherRecord &r : records) {
        if (!std::isnan(r.temperature_c)) {
            if (r.temperature_c > highest) highest = r.temperature_c;
            if (r.temperature_c < lowest)  lowest  = r.temperature_c;
        }
    }
    if (std::isfinite(highest)) ui->highestTemp->display(highest);
    if (std::isfinite(lowest))  ui->lowestTemp->display(lowest);

    m_tempChart->setRecords(records);
    ui->temperatureBandWidget->setRecords(records);
}

// ─── Database worker callbacks ────────────────────────────────────────────────

void MainWindow::onDbLoadCompleted(QVector<WeatherRecord> records)
{
    if (records.isEmpty()) {
        statusBar()->showMessage(tr("No cached data — load a CSV file to begin."));
        return;
    }

    // Push records into the model and refresh all views.
    m_model->setRecords(std::move(records));
    populateUi(m_model->allRecords());
    statusBar()->showMessage(
        tr("Loaded %1 records from cache.").arg(m_model->recordCount()));
}

void MainWindow::onDbSaveCompleted(int count)
{
    statusBar()->showMessage(tr("Saved %1 records to cache.").arg(count), 5000);
}

void MainWindow::onDbError(QString message)
{
    statusBar()->showMessage(tr("Database error: %1").arg(message));
    qWarning() << "[WeatherDB]" << message;
}

// ─── Menu actions ─────────────────────────────────────────────────────────────

void MainWindow::on_actionLoad_CSV_triggered()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("Open CSV"), QString(), tr("CSV files (*.csv)"));
    if (path.isEmpty())
        return;

    QString err;
    if (!m_model->loadFromFile(path, &err)) {
        QMessageBox::critical(this, tr("Error"), err);
        return;
    }

    qDebug() << "Loaded" << m_model->recordCount() << "records from CSV";
    populateUi(m_model->allRecords());
    statusBar()->showMessage(
        tr("Loaded %1 records from CSV. Saving to cache…").arg(m_model->recordCount()));

    // Persist to SQLite in the background — does not block the UI.
    emit requestDbSave(m_model->allRecords());
}

