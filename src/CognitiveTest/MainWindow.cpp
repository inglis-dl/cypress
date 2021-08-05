#include "MainWindow.h"

CognitiveTest::CognitiveTest(InputsModel initialInputs, QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    inputs = initialInputs;

    // Load Json into inputs
    CognitiveIO::ReadJsonInputs(jsonConfigPath, &inputs);

    // Run Cognitive Noodle Test
    ChoiceReactionTest::Run(&inputs);

    // Move results file to desktop
    // TODO: Figure out what to do with results
    ChoiceReactionTest::MoveResultsFile(inputs.path);
}
