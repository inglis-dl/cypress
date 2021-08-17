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

private:
    Ui::CognitiveTestClass ui;
    InputsModel inputs;
};
