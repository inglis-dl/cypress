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
    CognitiveTest(InputsModel initialInputs, QWidget* parent = Q_NULLPTR);
    int GetExitCode() { return exitCode; }

private:
    Ui::CognitiveTestClass ui;
    InputsModel inputs;
    QString jsonConfigPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/CognitiveTestInput.json";
    int exitCode = ExitCodes::Continue;
};
