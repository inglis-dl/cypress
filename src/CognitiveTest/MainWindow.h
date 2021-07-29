#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "InputsModel.h"
#include "ChoiceReactionTest.h"
#include "CognitiveIO.h"

#include <QString>
#include <QStandardPaths>

class CognitiveTest : public QMainWindow
{
    Q_OBJECT

public:
    CognitiveTest(QWidget *parent = Q_NULLPTR);

private:
    Ui::CognitiveTestClass ui;
    InputsModel inputs;
    QString jsonConfigPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/CognitiveTestInput.json";
};
