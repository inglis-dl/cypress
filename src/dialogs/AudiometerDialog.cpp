#include "AudiometerDialog.h"
#include "../managers/AudiometerManager.h"

#include <QDebug>
#include <QMessageBox>
#include <QTimeLine>

AudiometerDialog::AudiometerDialog(QWidget *parent)
    : DialogBase(parent)
    , ui(new Ui::AudiometerDialog)
{
    ui->setupUi(this);
    m_manager.reset(new AudiometerManager(this));
}

AudiometerDialog::~AudiometerDialog()
{
    delete ui;
}

void AudiometerDialog::initializeModel()
{
    for(int col=0;col<m_manager->getNumberOfModelColumns();col++)
    {
      for(int row=0;row<m_manager->getNumberOfModelRows();row++)
      {
        QStandardItem* item = new QStandardItem();
        m_model.setItem(row,col,item);
      }
    }
    m_model.setHeaderData(0,Qt::Horizontal,"Left",Qt::DisplayRole);
    m_model.setHeaderData(1,Qt::Horizontal,"Right",Qt::DisplayRole);
    ui->testdataTableView->setModel(&m_model);

    ui->testdataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->testdataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->verticalHeader()->hide();
}

// set up signal slot connections between GUI front end
// and device management back end
//
void AudiometerDialog::initializeConnections()
{
  QSharedPointer<AudiometerManager> derived = m_manager.staticCast<AudiometerManager>();

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
          ui->barcodeLineEdit->repaint();

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
  connect(derived.get(), &AudiometerManager::scanningDevices,
          ui->deviceComboBox, &QComboBox::clear);

  // Update the drop down list as devices are discovered during scanning
  //
  connect(derived.get(), &AudiometerManager::deviceDiscovered,
          this,[this](const QString &label){
      int index = ui->deviceComboBox->findText(label);
      bool oldState = ui->deviceComboBox->blockSignals(true);
      if(-1 == index)
      {
          ui->deviceComboBox->addItem(label);
      }
      ui->deviceComboBox->blockSignals(oldState);
  });

  connect(derived.get(), &AudiometerManager::deviceSelected,
          this,[this](const QString &label){
      if(label!=ui->deviceComboBox->currentText())
      {
          ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findText(label));
      }
  });

  // Prompt user to select a device from the drop down list when previously
  // cached device information in the ini file is unavailable or invalid
  //
  connect(derived.get(), &AudiometerManager::canSelectDevice,
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
          derived.get(),&AudiometerManager::selectDevice);

  // Select a device (serial port) from drop down list
  //
  connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::activated),
    this,[this, derived](int index){
      derived->selectDevice(ui->deviceComboBox->itemText(index));
  });

  // Ready to connect device
  //
  connect(derived.get(), &AudiometerManager::canConnectDevice,
          this,[this](){
      ui->connectButton->setEnabled(true);
      ui->disconnectButton->setEnabled(false);
      ui->measureButton->setEnabled(false);
      ui->saveButton->setEnabled(false);
  });

  // Connect to device
  //
  connect(ui->connectButton, &QPushButton::clicked,
          derived.get(), &AudiometerManager::connectDevice);

  // Connection is established: enable measurement requests
  //
  connect(m_manager.get(), &AudiometerManager::canMeasure,
          this,[this](){
      ui->connectButton->setEnabled(false);
      ui->disconnectButton->setEnabled(true);
      ui->measureButton->setEnabled(true);
      ui->saveButton->setEnabled(false);
  });

  // Disconnect from device
  //
  connect(ui->disconnectButton, &QPushButton::clicked,
          derived.get(), &AudiometerManager::disconnectDevice);

  // Request a measurement from the device
  //
  connect(ui->measureButton, &QPushButton::clicked,
        derived.get(), &AudiometerManager::measure);

  // Update the UI with any data
  //
  connect(m_manager.get(), &AudiometerManager::dataChanged,
          this,[this](){
      m_manager->buildModel(&m_model);

      auto h = ui->testdataTableView->horizontalHeader();
      QSize ts_pre = ui->testdataTableView->size();
      h->resizeSections(QHeaderView::ResizeToContents);
      ui->testdataTableView->setColumnWidth(0,h->sectionSize(0));
      ui->testdataTableView->setColumnWidth(1,h->sectionSize(1));
      ui->testdataTableView->resize(
                  h->sectionSize(0)+h->sectionSize(1)+1,
                  8*ui->testdataTableView->rowHeight(0)+1+
                  h->height());
      QSize ts_post = ui->testdataTableView->size();
      int dx = ts_post.width()-ts_pre.width();
      int dy = ts_post.height()-ts_pre.height();
      this->resize(this->width()+dx,this->height()+dy);
  });

  // All measurements received: enable write test results
  //
  connect(m_manager.get(), &AudiometerManager::canWrite,
          this,[this](){
      ui->saveButton->setEnabled(true);
  });

  // Write test data to output
  //
  connect(ui->saveButton, &QPushButton::clicked,
    this, &AudiometerDialog::writeOutput);

  // Close the application
  //
  connect(ui->closeButton, &QPushButton::clicked,
          this, &AudiometerDialog::close);

  // Read inputs, such as interview barcode
  //
  readInput();
}

QString AudiometerDialog::getVerificationBarcode() const
{
  return ui->barcodeLineEdit->text().simplified().remove(" ");
}
