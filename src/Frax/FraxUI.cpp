#include "FraxUI.h"

#include <QString>

void FraxUI::SetInputs(Ui::FraxClass* ui, InputsModel* inputs)
{
    ui->val1->setText(inputs->val1);
    ui->val2->setText(QString::number(inputs->val2));
    ui->val3->setText(QString::number(inputs->val3));
    ui->val4->setText(QString::number(inputs->val4));
    ui->val5->setText(QString::number(inputs->val5));
    ui->val6->setText(QString::number(inputs->val6));
    ui->val7->setText(QString::number(inputs->val7));
    ui->val8->setText(QString::number(inputs->val8));
    ui->val9->setText(QString::number(inputs->val9));
    ui->val10->setText(QString::number(inputs->val10));
    ui->val11->setText(QString::number(inputs->val11));
    ui->val12->setText(QString::number(inputs->val12));
    ui->dxaHipTScore->setText(QString::number(inputs->dxaHipTScore));
}

void FraxUI::SetOutputs(Ui::FraxClass* ui, OutputsModel* outputs)
{
    // TODO: Probably should double check none have changed
    SetInputs(ui, outputs);

    // Load outputs on UI
    ui->fracRisk1->setText(QString::number(outputs->fracRisk1));
    ui->fracRisk2->setText(QString::number(outputs->fracRisk2));
    ui->fracRisk3->setText(QString::number(outputs->fracRisk3));
    ui->fracRisk4->setText(QString::number(outputs->fracRisk4));
}
