#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "WeatherModel.h"
#include <QFileDialog>
#include <QMessageBox>

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

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionLoad_CSV_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, "Open CSV", "C:\\Users\\Administrator\\Documents\\qt-basics-13.05.2026", "CSV (*.csv)");
    if (!path.isEmpty()) {
        QString err;
        if (!m_model->loadFromFile(path, &err))
            QMessageBox::critical(this, "Error", err);
        else
            qDebug() << "Loaded" << m_model->recordCount() << "records";
    }
}

