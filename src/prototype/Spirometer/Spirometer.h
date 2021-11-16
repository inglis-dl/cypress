#pragma once

#include <QtWidgets/QDialog>
#include "ui_Spirometer.h"

#include "OnyxInXml.h"
#include "OnyxOutXml.h"
#include "EasyOnPcHelper.h"

#include <QDebug>

class Spirometer : public QDialog
{
    Q_OBJECT

public:
    Spirometer(QWidget *parent = Q_NULLPTR);
public slots:
    void OnStartClicked();
private:
    Ui::SpirometerClass ui;
    void SetupSlots();
};
