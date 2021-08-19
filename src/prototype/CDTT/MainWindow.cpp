#include "MainWindow.h"
#include "InputsModel.h"
#include "CDTTest.h"

CDTT::CDTT(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    InputsModel inputs;
    inputs.participantId = "98765432";
    inputs.path = "C:/Users/clsa/Downloads/CDTT-2018-07-22/CDTT-2018-07-22";

    CDTTest::Run(&inputs);
}
