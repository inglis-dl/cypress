#include "Spirometer.h"

Spirometer::Spirometer(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    SetupSlots();
}

void Spirometer::SetupSlots()
{
    // Connect ui buttons
    connect(ui.StartButton, SIGNAL(clicked()), this, SLOT(OnStartClicked()));
}

void Spirometer::OnStartClicked() {
    qDebug() << "Writing in file" << endl;
    /*OnyxInXml::Write();
    EasyOnPcHelper::ResetFiles();
    EasyOnPcHelper::LaunchEasyOnPc();*/
    OnyxOutXml::ReadImportantValues();
}
