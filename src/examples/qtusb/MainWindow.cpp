#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    for(auto id, m_usb.devices())
    {
        QString line = "usb device id " + id;
        ui->listWidget->addItem(line);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

