#include "MainWindow.h"

CognitiveTest::CognitiveTest(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Load Json into inputs
    inputs = CognitiveIO::ReadInputs(jsonConfigPath);

    // Run Cognitive Noodle Test
    NoodleTest::Run(&inputs);

    // Move results file to desktop
    // TODO: Figure out what to do with results
    NoodleTest::MoveResultsFile(inputs.path);

}
