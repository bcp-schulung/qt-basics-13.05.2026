#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "WeatherModel.h"
#include "TemperatureAreaChart.h"

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

private slots:
    void on_actionLoad_CSV_triggered();

private:
    Ui::MainWindow *ui;
    WeatherModel *m_model;
    WeatherProxyModel *m_proxy;
    TemperatureAreaChart *m_tempChart;
};
#endif // MAINWINDOW_H
