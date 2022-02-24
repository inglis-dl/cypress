#include "ThermometerDialog.h"
#include "../managers/BluetoothLEManager.h"

#include <QDebug>
#include <QMessageBox>

ThermometerDialog::ThermometerDialog(QWidget *parent)
    : DialogBase(parent)
    , ui(new Ui::ThermometerDialog)
{
    ui->setupUi(this);
    m_manager.reset(new BluetoothLEManager(this));
}

ThermometerDialog::~ThermometerDialog()
{
    delete ui;
}

void ThermometerDialog::initializeModel()
{
    // allocate 1 column x 1 rows of temperature measurement items
    //
    for(int row = 0; row < m_manager->getNumberOfModelRows(); row++)
    {
      QStandardItem* item = new QStandardItem();
      m_model.setItem(row,0,item);
    }
    m_model.setHeaderData(0,Qt::Horizontal,"Temperature Tests",Qt::DisplayRole);
    ui->testdataTableView->setModel(&m_model);

    ui->testdataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->testdataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->verticalHeader()->hide();
}

// set up signal slot connections between GUI front end
// and device management back end
//
void ThermometerDialog::initializeConnections()
{
  QSharedPointer<BluetoothLEManager> derived =
    m_manager.staticCast<BluetoothLEManager>();

  // Disable all buttons by default
  //
  for(auto&& x : this->findChildren<QPushButton *>())
  {
      x->setEnabled(false);

      // disable enter key press event passing onto auto focus buttons
      //
      x->setDefault(false);
      x->setAutoDefault(false);
  }

  // Close the application
  //
  ui->closeButton->setEnabled(true);

  // Relay messages from the manager to the status bar
  //
  connect(m_manager.get(),&ManagerBase::message,
          ui->statusBar, &QStatusBar::showMessage, Qt::DirectConnection);

  // Every instrument stage launched by an interviewer requires input
  // of the interview barcode that accompanies a participant.
  // The expected barcode is passed from upstream via .json file.
  // In simulate mode this value is ignored and a default barcode "00000000" is
  // assigned instead.
  // In production mode the input to the barcodeLineEdit is verified against
  // the content held by the manager and a message or exception is thrown accordingly
  //
  // TODO: for DCS interviews, the first digit corresponds the the wave rank
  // for inhome interviews there is a host dependent prefix before the barcode
  //
  if(CypressConstants::RunMode::Simulate == m_mode)
  {
    ui->barcodeWidget->setBarcode("00000000");
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

    // Scan for bluetooth low energy peripheral devices
    //
    ui->scanButton->setEnabled(true);

    connect(ui->scanButton, &QPushButton::clicked,
            derived.get(), &BluetoothLEManager::start);

    connect(derived.get(), &BluetoothLEManager::scanningDevices,
            ui->deviceComboBox, &QComboBox::clear);

    // Update the drop down list as devices are discovered during scanning
    //
    connect(derived.get(), &BluetoothLEManager::deviceDiscovered,
            this, [this](const QString &label){
        int index = ui->deviceComboBox->findText(label);
        bool oldState = ui->deviceComboBox->blockSignals(true);
        if(-1 == index)
        {
            ui->deviceComboBox->addItem(label);
        }
        ui->deviceComboBox->blockSignals(oldState);
    });

    connect(derived.get(), &BluetoothLEManager::deviceSelected,
            this,[this](const QString &label){
        if(label!=ui->deviceComboBox->currentText())
        {
            ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findText(label));
        }
    });

    // Prompt user to select a device from the drop down list when previously
    // cached device information in the ini file is unavailable or invalid
    //
    connect(derived.get(), &BluetoothLEManager::canSelectDevice,
            this,[this](){
        QMessageBox::warning(
          this, QApplication::applicationName(),
          tr("Double click the bluetooth thermometer from the list.  If the device "
          "is not in the list, turn on the device and click Scan or quit the application and check that the bluetooth adapeter is "
          "working and pair the thermometer to it before running this application."));
    });

    // Select a bluetooth low energy device from the drop down list
    //
    connect(ui->deviceComboBox, &QComboBox::currentTextChanged,
            derived.get(),&BluetoothLEManager::selectDevice);

    // Select a device (serial port) from drop down list
    //
    connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::activated),
      this,[this,derived](int index){
        derived->selectDevice(ui->deviceComboBox->itemText(index));
    });

    // Ready to connect device
    //
    connect(derived.get(), &BluetoothLEManager::canConnectDevice,
            this,[this](){
        ui->connectButton->setEnabled(true);
        ui->disconnectButton->setEnabled(false);
        ui->measureButton->setEnabled(false);
        ui->saveButton->setEnabled(false);
    });

    // Connect to device
    //
    connect(ui->connectButton, &QPushButton::clicked,
          derived.get(), &BluetoothLEManager::connectDevice);

    // Connection is established: enable measurement requests
    //
    connect(m_manager.get(), &BluetoothLEManager::canMeasure,
            this,[this](){
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
        ui->measureButton->setEnabled(true);
        ui->saveButton->setEnabled(false);
    });

    // Disconnect from device
    //
    connect(ui->disconnectButton, &QPushButton::clicked,
            derived.get(), &BluetoothLEManager::disconnectDevice);

    // Request a measurement from the device
    //
    connect(ui->measureButton, &QPushButton::clicked,
          derived.get(), &BluetoothLEManager::measure);

    // Update the UI with any data
    //
    connect(m_manager.get(), &BluetoothLEManager::dataChanged,
            this,[this](){
        m_manager->buildModel(&m_model);

        QSize ts_pre = ui->testdataTableView->size();
        ui->testdataTableView->setColumnWidth(0,ui->testdataTableView->size().width()-2);
        ui->testdataTableView->resize(
                    ui->testdataTableView->width(),
                    2*(ui->testdataTableView->rowHeight(0) + 1) +
                    ui->testdataTableView->horizontalHeader()->height());
        QSize ts_post = ui->testdataTableView->size();
        int dx = ts_post.width()-ts_pre.width();
        int dy = ts_post.height()-ts_pre.height();
        this->resize(this->width()+dx,this->height()+dy);
    });

    // All measurements received: enable write test results
    //
    connect(m_manager.get(), &BluetoothLEManager::canWrite,
            this,[this](){
       ui->saveButton->setEnabled(true);
    });

    // Write test data to output
    //
    connect(ui->saveButton, &QPushButton::clicked,
      this, &ThermometerDialog::writeOutput);

    // Close the application
    //
    connect(ui->closeButton, &QPushButton::clicked,
            this, &ThermometerDialog::close);

    // Read inputs, such as interview barcode
    //
    readInput();
}

QString ThermometerDialog::getVerificationBarcode() const
{
  return ui->barcodeWidget->barcode();
}

void ThermometerDialog::setVerificationBarcode(const QString &barcode)
{
    ui->barcodeWidget->setBarcode(barcode);
}
