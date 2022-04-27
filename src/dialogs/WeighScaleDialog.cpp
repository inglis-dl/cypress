#include "WeighScaleDialog.h"
#include "../managers/WeighScaleManager.h"

#include <QDebug>
#include <QMessageBox>

WeighScaleDialog::WeighScaleDialog(QWidget *parent)
    : DialogBase(parent)
    , ui(new Ui::WeighScaleDialog)
{
    ui->setupUi(this);
    m_manager.reset(new WeighScaleManager(this));
}

WeighScaleDialog::~WeighScaleDialog()
{
    delete ui;
}

void WeighScaleDialog::initializeModel()
{
    m_manager.get()->initializeModel();
    ui->measureWidget->initialize(m_manager.get()->getModel());
}

void WeighScaleDialog::initializeConnections()
{
  QSharedPointer<WeighScaleManager> derived =
    m_manager.staticCast<WeighScaleManager>();

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
  connect(derived.get(), &WeighScaleManager::scanningDevices,
          ui->deviceComboBox, &QComboBox::clear);

  // Update the drop down list as devices are discovered during scanning
  //
  connect(derived.get(), &WeighScaleManager::deviceDiscovered,
          this, [this](const QString &label){
      int index = ui->deviceComboBox->findText(label);
      bool oldState = ui->deviceComboBox->blockSignals(true);
      if(-1 == index)
      {
          ui->deviceComboBox->addItem(label);
      }
      ui->deviceComboBox->blockSignals(oldState);
  });

  connect(derived.get(), &WeighScaleManager::deviceSelected,
          this,[this](const QString &label){
      if(label != ui->deviceComboBox->currentText())
      {
          ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findText(label));
      }
  });

  // Prompt user to select a device from the drop down list when previously
  // cached device information in the ini file is unavailable or invalid
  //
  connect(derived.get(), &WeighScaleManager::canSelectDevice,
          this,[this](){
      QMessageBox::warning(
        this, QApplication::applicationName(),
        tr("Select the port from the list.  If the device "
        "is not in the list, quit the application and check that the port is "
        "working and connect the audiometer to it before running this application."));
  });

  // Select a device (serial port) from drop down list
  //
  connect(ui->deviceComboBox, &QComboBox::currentTextChanged,
          derived.get(),&WeighScaleManager::selectDevice);

  // Select a device (serial port) from drop down list
  //
  connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::activated),
    this,[this,derived](int index){
      derived->selectDevice(ui->deviceComboBox->itemText(index));
  });

  // Ready to connect device
  //
  connect(derived.get(), &WeighScaleManager::canConnectDevice,
          this,[this](){
      ui->connectButton->setEnabled(true);
      ui->disconnectButton->setEnabled(false);

      //TODO: fix MeasureWidget segfault when adding extra zero button
      // ui->zeroButton->setEnabled(false);
      //
      ui->measureWidget->disableMeasure();
  });

  // Connect to device
  //
  connect(ui->connectButton, &QPushButton::clicked,
          derived.get(), &WeighScaleManager::connectDevice);

  // Connection is established: enable measurement requests
  //
  connect(derived.get(), &WeighScaleManager::canMeasure,
          ui->measureWidget, &MeasureWidget::enableMeasure);

  connect(m_manager.get(), &WeighScaleManager::canMeasure,
          this,[this](){
      ui->connectButton->setEnabled(false);
      ui->disconnectButton->setEnabled(true);
  });

  // Disconnect from device
  //
  connect(ui->disconnectButton, &QPushButton::clicked,
          derived.get(), &WeighScaleManager::disconnectDevice);

  //TODO: fix MeasureWidget segfault when adding extra zero button
  /*      this code should be integrated into MeasureWidget
  // Zero the scale
  //
  connect(ui->zeroButton, &QPushButton::clicked,
        derived.get(), &WeighScaleManager::zeroDevice);
  */

  // Request a measurement from the device
  //
  connect(ui->measureWidget, &MeasureWidget::measure,
        derived.get(), &WeighScaleManager::measure);

  // Update the UI with any data
  //
  connect(derived.get(), &WeighScaleManager::dataChanged,
      ui->measureWidget, &MeasureWidget::updateModelView);

  // All measurements received: enable write test results
  //
  connect(derived.get(), &WeighScaleManager::canWrite,
      ui->measureWidget, &MeasureWidget::enableWriteToFile);

  // Write test data to output
  //
  connect(ui->measureWidget, &MeasureWidget::writeToFile,
      this, &DialogBase::writeOutput);

  // Close the application
  //
  connect(ui->measureWidget, &MeasureWidget::closeApplication,
      this, &DialogBase::close);
}

QString WeighScaleDialog::getVerificationBarcode() const
{
  return ui->barcodeWidget->barcode();
}

void WeighScaleDialog::setVerificationBarcode(const QString &barcode)
{
    ui->barcodeWidget->setBarcode(barcode);
}
