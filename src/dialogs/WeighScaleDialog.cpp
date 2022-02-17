#include "WeighScaleDialog.h"
#include "../managers/WeighScaleManager.h"

#include <QDebug>
#include <QMessageBox>
#include <QTimeLine>

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
    qDebug() <<"weigh scale dialog init model";
    // allocate 1 column x 2 rows of weight measurement items
    //
    for(int row=0;row<2;row++)
    {
      QStandardItem* item = new QStandardItem();
      m_model.setItem(row,0,item);
    }
    m_model.setHeaderData(0,Qt::Horizontal,"Weight Tests",Qt::DisplayRole);
    ui->testdataTableView->setModel(&m_model);

    ui->testdataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->testdataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->verticalHeader()->hide();
    qDebug() <<"weigh scale dialog init model finished";

}

void WeighScaleDialog::initializeConnections()
{
  QSharedPointer<WeighScaleManager> derived =
    m_manager.staticCast<WeighScaleManager>();

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
  if(CypressConstants::Mode::Simulate == m_mode)
  {
    ui->barcodeLineEdit->setText("00000000");
  }

  //TODO: handle the case for in home DCS visits where
  // the barcode is prefixed with a host name code
  //
  QRegExp rx("\\d{8}");
  QRegExpValidator *v_barcode = new QRegExpValidator(rx);
  ui->barcodeLineEdit->setValidator(v_barcode);

  connect(ui->barcodeLineEdit, &QLineEdit::returnPressed,
          this,[this](){
      bool valid = false;
      if(m_inputData.contains("barcode"))
      {
          QString str = ui->barcodeLineEdit->text().simplified();
          str.replace(" ","");
          valid = str == m_inputData["barcode"].toString();
      }
      auto p = this->findChild<QTimeLine *>("timer");
      if(valid)
      {
          p->stop();
          p->setCurrentTime(0);
          auto p = ui->barcodeLineEdit->palette();
          p.setBrush(QPalette::Base,QBrush(QColor(0,255,0,128)));
          ui->barcodeLineEdit->setPalette(p);

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

  auto timeLine = new QTimeLine(2000,this);
  timeLine->setFrameRange(0,255);
  timeLine->setLoopCount(0);
  timeLine->setObjectName("timer");
  connect(timeLine, &QTimeLine::frameChanged,
          this,[this](int frame){
      auto p = ui->barcodeLineEdit->palette();
      p.setBrush(QPalette::Base,QBrush(QColor(255,255,0,frame)));
      ui->barcodeLineEdit->setPalette(p);
  });
  connect(timeLine, &QTimeLine::finished, timeLine, &QTimeLine::deleteLater);
  timeLine->start();

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
      if(label!=ui->deviceComboBox->currentText())
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
      ui->zeroButton->setEnabled(false);
      ui->disconnectButton->setEnabled(false);
      ui->measureButton->setEnabled(false);
      ui->saveButton->setEnabled(false);
  });

  // Connect to device
  //
  connect(ui->connectButton, &QPushButton::clicked,
          derived.get(), &WeighScaleManager::connectDevice);

  // Connection is established: enable measurement requests
  //
  connect(m_manager.get(), &WeighScaleManager::canMeasure,
          this,[this](){
      ui->connectButton->setEnabled(false);
      ui->disconnectButton->setEnabled(true);
      ui->zeroButton->setEnabled(true);
      ui->measureButton->setEnabled(true);
      ui->saveButton->setEnabled(false);
  });

  // Disconnect from device
  //
  connect(ui->disconnectButton, &QPushButton::clicked,
          derived.get(), &WeighScaleManager::disconnectDevice);

  // Zero the scale
  //
  connect(ui->zeroButton, &QPushButton::clicked,
        derived.get(), &WeighScaleManager::zeroDevice);

  // Request a measurement from the device
  //
  connect(ui->measureButton, &QPushButton::clicked,
        derived.get(), &WeighScaleManager::measure);

  // Update the UI with any data
  //
  connect(m_manager.get(), &WeighScaleManager::dataChanged,
          this,[this](){
      m_manager->buildModel(&m_model);

      QSize ts_pre = ui->testdataTableView->size();
      ui->testdataTableView->setColumnWidth(0,ui->testdataTableView->size().width()-2);
      ui->testdataTableView->resize(
                  ui->testdataTableView->width(),
                  2*(ui->testdataTableView->rowHeight(0)+1)+
                  ui->testdataTableView->horizontalHeader()->height());
      QSize ts_post = ui->testdataTableView->size();
      int dx = ts_post.width()-ts_pre.width();
      int dy = ts_post.height()-ts_pre.height();
      this->resize(this->width()+dx,this->height()+dy);
  });

  // All measurements received: enable write test results
  //
  connect(m_manager.get(), &WeighScaleManager::canWrite,
          this,[this](){
      ui->saveButton->setEnabled(true);
  });

  // Write test data to output
  //
  connect(ui->saveButton, &QPushButton::clicked,
    this, &WeighScaleDialog::writeOutput);

  // Close the application
  //
  connect(ui->closeButton, &QPushButton::clicked,
          this, &WeighScaleDialog::close);

  // Read inputs, such as interview barcode
  //
  readInput();
}

QString WeighScaleDialog::getVerificationBarcode() const
{
  return ui->barcodeLineEdit->text().simplified().remove(" ");
}
