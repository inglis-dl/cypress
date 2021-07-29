#include "MainWindow.h"

Frax::Frax(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Read inputs from json and display
    inputs = FraxIO::ReadInputs(jsonInputsPath);
    FraxUI::SetInputs(&ui, &inputs);
}

void Frax::CalculateClicked()
{
    // Save example input.txt to be put back later
    QFile::rename(GetInputFilePath(), GetSavedInputFilePath());

    // Create new input.txt from our inputs
    FraxIO::CreateInputTxt(&inputs);

    // Run blackbox.exe
    QProcess process;
    process.start(GetBlackBoxExeFilePath());
    process.waitForFinished(2000);
    process.close();

    // read output.txt into outputs and display
    outputs = FraxIO::ReadOutputs(GetOutputFilePath());
    FraxUI::SetOutputs(&ui, &outputs);

    // restore files to original state
    QFile::remove(GetInputFilePath());
    QFile::remove(GetOutputFilePath());
    QFile::rename(GetSavedInputFilePath(), GetInputFilePath());
}
