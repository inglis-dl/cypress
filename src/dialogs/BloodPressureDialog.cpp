#include "BloodPressureDialog.h"
#include "../managers/BloodPressureManager.h"

#include <QDebug>
#include <QMessageBox>

BloodPressureDialog::BloodPressureDialog(QWidget *parent)
    : DialogBase(parent)
    , ui(new Ui::BloodPressureDialog)
{
    ui->setupUi(this);
    m_manager.reset(new BloodPressureManager(this));
}

BloodPressureDialog::~BloodPressureDialog()
{
    delete ui;
}

void BloodPressureDialog::initializeModel()
{
    m_manager.get()->initializeModel();
    ui->measureWidget->initialize(m_manager.get()->getModel());
}

void BloodPressureDialog::initializeConnections()
{
    QSharedPointer<BloodPressureManager> derived = m_manager.staticCast<BloodPressureManager>();

    // The qcombobox automatically selects the first item in the list. So a blank entry is
    // added to display that an option has not been picked yet. The blank entry is treated
    // as not being a valid option by the rest of this code. If there was no blank entry, then
    // another option would be set by default and users may forget to change the default option
    //
    QStringList bandList = (QStringList()<<""<<"small"<<"medium"<<"large"<<"x-large");
    ui->armBandSizeComboBox->addItems(bandList);
    ui->armBandSizeComboBox->setCurrentIndex(0);

    QStringList armList = (QStringList()<<""<<"left"<<"right");
    ui->armComboBox->addItems(armList);
    ui->armComboBox->setCurrentIndex(0);

    // Update cuff size selected by user
    //
    connect(ui->armBandSizeComboBox, &QComboBox::currentTextChanged,
        derived.get(), &BloodPressureManager::setCuffSize);

    // Update arm used selected by user
    //
    connect(ui->armComboBox, &QComboBox::currentTextChanged,
            derived.get(), &BloodPressureManager::setSide);

    connect(derived.get(), &BloodPressureManager::cuffSizeChanged,
            ui->armBandSizeComboBox, &QComboBox::setCurrentText);

    connect(derived.get(), &BloodPressureManager::sideChanged,
            ui->armComboBox, &QComboBox::setCurrentText);

    // Disable all buttons by default
    //
    foreach(auto button, this->findChildren<QPushButton *>())
    {
        if("Close" != button->text())
          button->setEnabled(false);

        // disable enter key press event passing onto auto focus buttons
        //
        button->setDefault(false);
        button->setAutoDefault(false);
    }

    // Relay messages from the manager to the status bar
    //
    connect(m_manager.get(),&ManagerBase::message,
            ui->statusBar, &QStatusBar::showMessage, Qt::DirectConnection);

    // Every instrument stage launched by an interviewer requires input
    // of the interview barcode that accompanies a participant.
    // The expected barcode is passed from upstream via .json file.
    // In simulate mode this value is ignored and a default barcode Constants::DefaultBarcode is
    // assigned instead.
    // In production mode the input to the barcodeLineEdit is verified against
    // the content held by the manager and a message or exception is thrown accordingly
    //
    // TODO: for DCS interviews, the first digit corresponds the the wave rank
    // for inhome interviews there is a host dependent prefix before the barcode
    //
    if(Constants::RunMode::modeSimulate == m_mode)
    {
      ui->barcodeWidget->setBarcode(Constants::DefaultBarcode);
    }

    connect(ui->barcodeWidget,&BarcodeWidget::validated,
            this,[this](const bool& valid)
      {
        if(valid)
        {
            // launch the manager
            //
            this->run();
        }
        else
        {
            QMessageBox::critical(
              this, QApplication::applicationName(),
              tr("The input does not match the expected barcode for this participant."));
        }
    });

    // Scan for devices
    //
    connect(derived.get(), &BloodPressureManager::scanningDevices,
            ui->deviceComboBox, &QComboBox::clear);

    // Update the drop down list as devices are discovered during scanning
    //
    connect(derived.get(), &BloodPressureManager::deviceDiscovered,
            this,[this](const QString &label){
        int index = ui->deviceComboBox->findText(label);
        bool oldState = ui->deviceComboBox->blockSignals(true);
        if(-1 == index)
        {
            ui->deviceComboBox->addItem(label);
        }
        ui->deviceComboBox->blockSignals(oldState);
    });

    connect(derived.get(), &BloodPressureManager::deviceSelected,
            this,[this](const QString &label){
        if(label != ui->deviceComboBox->currentText())
        {
            ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findText(label));
        }
    });

    // Prompt user to select a device from the drop down list when previously
    // cached device information in the ini file is unavailable or invalid
    //
    connect(derived.get(), &BloodPressureManager::canSelectDevice,
            this,[this](){
        QMessageBox::warning(
          this, QApplication::applicationName(),
          tr("Select the device from the list.  If the device "
          "is not in the list, quit the application and check that the usb port is "
          "working and connect the blood pressure monitor to it before running this application."));
    });

    // Select a device from drop down list
    //
    connect(ui->deviceComboBox, &QComboBox::currentTextChanged,
            derived.get(), &BloodPressureManager::selectDevice);

    // Select a device from drop down list
    //
    connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::activated),
      this,[this, derived](int index){
        derived->selectDevice(ui->deviceComboBox->itemText(index));
    });

    // Ready to connect device
    //
    connect(derived.get(), &BloodPressureManager::canConnectDevice,
            this,[this]() {
        ui->connectButton->setEnabled(true);
        ui->disconnectButton->setEnabled(false);
        ui->measureWidget->disableMeasure();
    });

    // Connect to the device (bpm)
    //
    connect(ui->connectButton, &QPushButton::clicked,
        derived.get(), &BloodPressureManager::connectDevice);

    // Connection is established: enable measurement requests
    //
    connect(derived.get(), &BloodPressureManager::canMeasure,
            this,[this](){
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
        ui->measureWidget->disableMeasure();

        // Remove blank enty from selection, so that user cannot change answer to blank
        // The user will still be able to change their selection if they make a mistake
        // But they will be forced to chose a valid option
        //
        ui->armBandSizeComboBox->removeItem(ui->armBandSizeComboBox->findText(""));
        ui->armComboBox->removeItem(ui->armComboBox->findText(""));
    });

    connect(derived.get(), &BloodPressureManager::canMeasure,
            this,[this](){
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
    });

    // Disconnect from device
    //
    connect(ui->disconnectButton, &QPushButton::clicked,
            derived.get(), &BloodPressureManager::disconnectDevice);

    // Request a measurement from the device
    //
    connect(ui->measureWidget, &MeasureWidget::measure,
          derived.get(), &BloodPressureManager::measure);

    // Update the UI with any data
    //
    connect(derived.get(), &BloodPressureManager::dataChanged,
        ui->measureWidget, &MeasureWidget::updateModelView);

    // All measurements received: enable write test results
    //
    connect(derived.get(), &BloodPressureManager::canWrite,
        ui->measureWidget, &MeasureWidget::enableWriteToFile);

    // Write test data to output
    //
    connect(ui->measureWidget, &MeasureWidget::writeToFile,
        this, &BloodPressureDialog::writeOutput);

    // Close the application
    //
    connect(ui->measureWidget, &MeasureWidget::closeApplication,
        this, &BloodPressureDialog::close);
  }

QString BloodPressureDialog::getVerificationBarcode() const
{
  return ui->barcodeWidget->barcode();
}

void BloodPressureDialog::setVerificationBarcode(const QString &barcode)
{
    ui->barcodeWidget->setBarcode(barcode);
}
