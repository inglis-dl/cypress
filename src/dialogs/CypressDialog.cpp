#include "CypressDialog.h"
#include <QVBoxLayout>
#include <QFormBuilder>
#include <QFile>
#include <QPushButton>
#include <QDebug>

CypressDialog::CypressDialog(QWidget *parent) : QDialog(parent)
{
}

CypressDialog::~CypressDialog()
{
}

void CypressDialog::initialize(const CypressApplication::TestType &type)
{
    QString ui_name;
    QString ui_title;
    switch(type)
    {
      case CypressApplication::TestType::Weight:
        ui_name = "weighscale.ui";
        ui_title = "Weight Measurement";
        break;
      case CypressApplication::TestType::BodyComposition:
        ui_name = "bodycomposition.ui";
        ui_title = "Body Composition Measurement";
        break;
      case CypressApplication::TestType::Hearing:
        ui_name = "audiometer.ui";
        ui_title = "Hearing Measurement";
        break;
      case CypressApplication::TestType::ChoiceReaction:
        ui_name = "choicereaction.ui";
        ui_title = "Choice Reacation Test";
        break;
      case CypressApplication::TestType::Temperature:
        ui_name = "thermometer.ui";
        ui_title = "Temperature Measurement";
        break;
      case CypressApplication::TestType::Frax:
        ui_name = "frax.ui";
        ui_title = "FRAX Measurement";
        break;
      case CypressApplication::TestType::CDTT:
        ui_title = "CDTT Test";
        break;
    }

    QFormBuilder builder;
    QFile file(":/dialogs/"+ui_name);
    file.open(QFile::ReadOnly);
    QWidget *widget = builder.load(&file,this);
    file.close();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(widget);
    this->setLayout(layout);

    this->setWindowTitle(ui_title);
    m_status.reset(this->findChild<QStatusBar*>("statusBar"));

    m_barcodeEdit.reset(this->findChild<QLineEdit*>("barcodeLineEdit"));


    // all ui frames have a close button and a save button
    //
    QPushButton* ui_closeButton = this->findChild<QPushButton*>("closeButton");
    if(nullptr!=ui_closeButton)
        qDebug() << ui_closeButton->objectName();

    QPushButton* ui_saveButton = this->findChild<QPushButton*>("saveButton");
    if(nullptr!=ui_saveButton)
        qDebug() << ui_saveButton->objectName();

    connect(ui_closeButton,&QPushButton::clicked,
            this,[this](){
        qDebug() << "closing the dialog";
        this->accept();
    });

    connect(ui_saveButton,&QPushButton::clicked,
            reinterpret_cast<CypressApplication*>(this->parent()), &CypressApplication::writeOutput);

    // construct the logic between the manager and the ui
}

QString CypressDialog::getBarcode() const
{
   return m_barcodeEdit->text();
}

void CypressDialog::setStatusMessage(const QString &message)
{
  m_status->showMessage(message);
}

