#include "MainWindow.h"

CognitiveTest::CognitiveTest(InputsModel initialInputs, QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    inputs = initialInputs;

    // Run Cognitive Noodle Test
    ChoiceReactionTest::Run(&inputs);

    // Move results file to desktop
    // TODO: Figure out what to do with results
    ChoiceReactionTest::MoveResultsFile(inputs.CCBFolderPath);
}
