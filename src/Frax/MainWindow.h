#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "InputsModel.h"
#include "OutputsModel.h"
#include "FraxIO.h"
#include "FraxUI.h"

#include <QString>
#include <QProcess>
#include <QStandardPaths>

class Frax : public QMainWindow
{
    Q_OBJECT

public:
    Frax(QWidget *parent = Q_NULLPTR);

private:
    Ui::FraxClass ui;
    InputsModel inputs;
    OutputsModel outputs;

    // FilePaths
    QString jsonInputsPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/FraxTestinput.json";
    QString GetBlackBoxExeFilePath() { return inputs.fraxFolderPath + "/blackbox.exe"; }
    QString GetInputFilePath() { return inputs.fraxFolderPath + "/input.txt"; }
    QString GetSavedInputFilePath() { return inputs.fraxFolderPath + "/oldInput.txt"; }
    QString GetOutputFilePath() { return inputs.fraxFolderPath + "/output.txt"; }

private slots:
    void CalculateClicked();
};
