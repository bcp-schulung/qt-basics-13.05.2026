#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "WeatherModel.h"
#include "TemperatureAreaChart.h"
#include "WeatherDatabase.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

signals:
    // Forwarded to WeatherDatabaseWorker::save() via queued connection.
    void requestDbSave(QVector<WeatherRecord> records);

    // Forwarded to WeatherDatabaseWorker::clearCache() via queued connection.
    void requestDbClear();

private slots:
    void on_actionLoad_CSV_triggered();
    void on_actionClear_Cache_triggered();

    // Database worker callbacks (delivered on the UI thread via queued signals).
    void onDbLoadCompleted(QVector<WeatherRecord> records);
    void onDbSaveCompleted(int count);
    void onDbCacheCleared();
    void onDbError(QString message);

private:
    // Populates the model, stats widgets and charts from a record vector.
    void populateUi(const QVector<WeatherRecord> &records);

    Ui::MainWindow        *ui;
    WeatherModel          *m_model;
    WeatherProxyModel     *m_proxy;
    TemperatureAreaChart  *m_tempChart;

    QThread               *m_dbThread  = nullptr;
    WeatherDatabaseWorker *m_dbWorker  = nullptr;
};
#endif // MAINWINDOW_H
