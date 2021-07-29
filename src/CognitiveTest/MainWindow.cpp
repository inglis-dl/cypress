#include "MainWindow.h"

CognitiveTest::CognitiveTest(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Load Json into inputs
    inputs = CognitiveIO::ReadInputs(jsonConfigPath);

    // Run Cognitive Noodle Test
    ChoiceReactionTest::Run(&inputs);

    // Move results file to desktop
    // TODO: Figure out what to do with results
    ChoiceReactionTest::MoveResultsFile(inputs.path);

}
