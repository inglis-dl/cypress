#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

class CDTT : public QMainWindow
{
    Q_OBJECT

public:
    CDTT(QWidget *parent = Q_NULLPTR);

private:
    Ui::CDTTClass ui;
};
