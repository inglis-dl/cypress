#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BPTruBPM200.h"

class BPTruBPM200 : public QMainWindow
{
    Q_OBJECT

public:
    BPTruBPM200(QWidget *parent = Q_NULLPTR);

private:
    Ui::BPTruBPM200Class ui;
};
