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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionLoad_CSV_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, "Open CSV", {}, "CSV (*.csv)");
    if (!path.isEmpty()) {
        QString err;
        if (!m_model->loadFromFile(path, &err))
            QMessageBox::critical(this, "Error", err);
        else
            qDebug() << "Loaded" << m_model->recordCount() << "records";
    }
}

