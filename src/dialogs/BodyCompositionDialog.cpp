#include "BodyCompositionDialog.h"
#include "../managers/BodyCompositionManager.h"

#include <QDebug>
#include <QMessageBox>

BodyCompositionDialog::BodyCompositionDialog(QWidget *parent)
    : DialogBase(parent)
    , ui(new Ui::BodyCompositionDialog)
{
    ui->setupUi(this);
    m_manager.reset(new BodyCompositionManager(this));
}

BodyCompositionDialog::~BodyCompositionDialog()
{
    delete ui;
}

void BodyCompositionDialog::initializeModel()
{
    m_manager.get()->initializeModel();
    ui->measureWidget->initialize(m_manager.get()->getModel());
}

void BodyCompositionDialog::initializeConnections()
{
  QSharedPointer<BodyCompositionManager> derived =
    m_manager.staticCast<BodyCompositionManager>();

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
  connect(derived.get(), &BodyCompositionManager::scanningDevices,
          ui->deviceComboBox, &QComboBox::clear);

  // Update the drop down list as devices are discovered during scanning
  //
  connect(derived.get(), &BodyCompositionManager::deviceDiscovered,
          this, [this](const QString &label){
      int index = ui->deviceComboBox->findText(label);
      bool oldState = ui->deviceComboBox->blockSignals(true);
      if(-1 == index)
      {
          ui->deviceComboBox->addItem(label);
      }
      ui->deviceComboBox->blockSignals(oldState);
  });

  connect(derived.get(), &BodyCompositionManager::deviceSelected,
          this,[this](const QString &label){
      if(label != ui->deviceComboBox->currentText())
      {
          ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findText(label));
      }
  });

  // Prompt user to select a device from the drop down list when previously
  // cached device information in the ini file is unavailable or invalid
  //
  connect(derived.get(), &BodyCompositionManager::canSelectDevice,
          this,[this](){
      ui->statusBar->showMessage("Ready to select...");
      QMessageBox::warning(
        this, QApplication::applicationName(),
        tr("Select the port from the list or connect to the currently visible port in the list.  If the device "
        "is not in the list, quit the application and check that the port is "
        "working and connect the analyzer to it before running this application."));
  });

  // Select a device (serial port) from drop down list
  //
  connect(ui->deviceComboBox, &QComboBox::currentTextChanged,
          derived.get(),&BodyCompositionManager::selectDevice);

  // Select a device (serial port) from drop down list
  //
  connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::activated),
    this,[this,derived](int index){
      derived->selectDevice(ui->deviceComboBox->itemText(index));
  });

  // Ready to connect device
  //
  connect(derived.get(), &BodyCompositionManager::canConnectDevice,
          this,[this](){
      ui->connectButton->setEnabled(true);
      ui->disconnectButton->setEnabled(false);
      ui->resetButton->setEnabled(false);
      ui->setButton->setEnabled(false);
      ui->confirmButton->setEnabled(false);
      ui->measureWidget->disableMeasure();
  });

  // Connect to device
  //
  connect(ui->connectButton, &QPushButton::clicked,
          derived.get(), &BodyCompositionManager::connectDevice);

  // Connection is established: enable measurement requests
  //
  connect(derived.get(), &BodyCompositionManager::canMeasure,
          ui->measureWidget, &MeasureWidget::enableMeasure);

  connect(m_manager.get(), &BodyCompositionManager::canMeasure,
          this,[this](){
      ui->connectButton->setEnabled(false);
      ui->disconnectButton->setEnabled(true);
      ui->resetButton->setEnabled(true);
      ui->setButton->setEnabled(false);
      ui->confirmButton->setEnabled(true);
  });

  // Connection is established and a successful reset was done
  //
  connect(derived.get(), &BodyCompositionManager::canInput,
          this,[this](){
      ui->connectButton->setEnabled(false);
      ui->disconnectButton->setEnabled(true);
      ui->resetButton->setEnabled(true);
      ui->setButton->setEnabled(true);
      ui->confirmButton->setEnabled(false);
      ui->measureWidget->disableMeasure();
  });

  // A successful confirmation of all inputs was done
  //
  connect(derived.get(), &BodyCompositionManager::canConfirm,
          this,[this](){
      ui->connectButton->setEnabled(false);
      ui->disconnectButton->setEnabled(true);
      ui->resetButton->setEnabled(true);
      ui->setButton->setEnabled(true);
      ui->confirmButton->setEnabled(true);
      ui->measureWidget->disableMeasure();
  });

  // Disconnect from device
  //
  connect(ui->disconnectButton, &QPushButton::clicked,
          derived.get(), &BodyCompositionManager::disconnectDevice);

  // Reset the device (clear all input settings)
  //
  connect(ui->resetButton, &QPushButton::clicked,
        derived.get(), &BodyCompositionManager::resetDevice);

  // Set the inputs to the analyzer
  //
  connect(ui->setButton, &QPushButton::clicked,
           this,[this, derived](){
      QMap<QString,QVariant> inputs;
      inputs["equation"] = "westerner";

      inputs["measurement_system"] =
        "metricRadio" == ui->unitsGroup->checkedButton()->objectName() ?
          "metric" : "imperial";

      QString units = inputs["measurement system"].toString();

      inputs["gender"] =
        "maleRadio" == ui->genderGroup->checkedButton()->objectName() ?
          "male" : "female";

      QString s = ui->ageLineEdit->text().simplified();
      s = s.replace(" ","");
      inputs["age"] = s.toUInt();

      inputs["body_type"] =
        "standardRadio" == ui->bodyTypeGroup->checkedButton()->objectName() ?
          "standard" : "athlete";

      s = ui->heightLineEdit->text().simplified();
      s = s.replace(" ","");
      inputs["height"] = "metric" == units ? s.toUInt() :  s.toDouble();

      s = ui->tareWeightLineEdit->text().simplified();
      s = s.replace(" ","");
      inputs["tare_weight"] = s.toDouble();

      qDebug() << inputs;
      derived->updateInputData(inputs);
  });

  // Confirm inputs and check if measurement can proceed
  //
  connect(ui->confirmButton, &QPushButton::clicked,
        derived.get(), &BodyCompositionManager::confirmSettings);

  // Request a measurement from the device
  //
  connect(ui->measureWidget, &MeasureWidget::measure,
        derived.get(), &BodyCompositionManager::measure);

  // Update the UI with any data
  //
  connect(derived.get(), &BodyCompositionManager::dataChanged,
      ui->measureWidget, &MeasureWidget::updateModelView);

  // All measurements received: enable write test results
  //
  connect(derived.get(), &BodyCompositionManager::canWrite,
      ui->measureWidget, &MeasureWidget::enableWriteToFile);

  QIntValidator *v_age = new QIntValidator(this);
  v_age->setRange(
    BodyCompositionManager::AGE_MIN,
    BodyCompositionManager::AGE_MAX);
  ui->ageLineEdit->setValidator(v_age);

  // When the units are changed to imperial, the input field for
  // height input must change to accomodate 0.5" increments
  //
  connect(ui->unitsGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
          this,[this](QAbstractButton *button){
     ui->heightLineEdit->setText("");
     if("metricRadio" == button->objectName())
     {
         QIntValidator *v_ht = new QIntValidator(this);
         v_ht->setRange(
           BodyCompositionManager::HEIGHT_MIN_METRIC,
           BodyCompositionManager::HEIGHT_MAX_METRIC);
         ui->heightLineEdit->setValidator(v_ht);
     }
     else
     {
         QDoubleValidator *v_ht = new QDoubleValidator(this);
         v_ht->setRange(
           BodyCompositionManager::HEIGHT_MIN_IMPERIAL,
           BodyCompositionManager::HEIGHT_MAX_IMPERIAL);
         v_ht->setDecimals(1);
         ui->heightLineEdit->setValidator(v_ht);
     }
  });

  connect(derived.get(), &BodyCompositionManager::error,
          this, [this](const QString &error){
      ui->statusBar->showMessage(error);
      QMessageBox::warning(
        this, QApplication::applicationName(),
        tr("A fatal error occured while attempting a measurement and the "
           "device was disconnected. Turn the device on if it is off, re-connect, "
           "re-input and confirm the inputs."));
  });

  // Write test data to output
  //
  connect(ui->measureWidget, &MeasureWidget::writeToFile,
      this, &DialogBase::writeOutput);

  // Close the application
  //
  connect(ui->measureWidget, &MeasureWidget::closeApplication,
      this, &DialogBase::close);
}

QString BodyCompositionDialog::getVerificationBarcode() const
{
  return ui->barcodeWidget->barcode();
}

void BodyCompositionDialog::setVerificationBarcode(const QString &barcode)
{
    ui->barcodeWidget->setBarcode(barcode);
}
