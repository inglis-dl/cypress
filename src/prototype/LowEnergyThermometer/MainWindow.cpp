#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../../auxiliary/JsonSettings.h"

#include <QBitArray>
#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMetaEnum>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
    , m_verbose(false)
    , m_manager(this)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
  initializeModel();
  initializeConnections();
}

void MainWindow::initializeModel()
{
    // allocate 1 column x 1 rows of temperature measurement items
    //
    for(int row = 0; row < m_manager.getNumberOfModelRows(); row++)
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
void MainWindow::initializeConnections()
{
  // Disable all buttons by default
  //
    foreach(auto button, this->findChildren<QPushButton *>())
    {
        button->setEnabled(false);

        // disable enter key press event passing onto auto focus buttons
        //
        button->setDefault(false);
        button->setAutoDefault(false);
    }

  // Close the application
  //
  ui->closeButton->setEnabled(true);

  // Relay messages from the manager to the status bar
  //
  connect(&m_manager,&ManagerBase::message,
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

    // Scan for bluetooth low energy peripheral devices
    //
    ui->scanButton->setEnabled(true);

    connect(ui->scanButton, &QPushButton::clicked,
            &m_manager, &BluetoothLEManager::start);

    connect(&m_manager, &BluetoothLEManager::scanningDevices,
            ui->deviceComboBox, &QComboBox::clear);

    // Update the drop down list as devices are discovered during scanning
    //
    connect(&m_manager, &BluetoothLEManager::deviceDiscovered,
            this, [this](const QString &label){
        int index = ui->deviceComboBox->findText(label);
        bool oldState = ui->deviceComboBox->blockSignals(true);
        if(-1 == index)
        {
            ui->deviceComboBox->addItem(label);
        }
        ui->deviceComboBox->blockSignals(oldState);
    });

    connect(&m_manager, &BluetoothLEManager::deviceSelected,
            this,[this](const QString &label){
        if(label!=ui->deviceComboBox->currentText())
        {
            ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findText(label));
        }
    });

    // Prompt user to select a device from the drop down list when previously
    // cached device information in the ini file is unavailable or invalid
    //
    connect(&m_manager, &BluetoothLEManager::canSelectDevice,
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
            &m_manager,&BluetoothLEManager::selectDevice);

    // Select a device (serial port) from drop down list
    //
    connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::activated),
      this,[this](int index){
        m_manager.selectDevice(ui->deviceComboBox->itemText(index));
    });

    // Ready to connect device
    //
    connect(&m_manager, &BluetoothLEManager::canConnectDevice,
            this,[this](){
        ui->connectButton->setEnabled(true);
        ui->disconnectButton->setEnabled(false);
        ui->measureButton->setEnabled(false);
        ui->saveButton->setEnabled(false);
    });

    // Connect to device
    //
    connect(ui->connectButton, &QPushButton::clicked,
          &m_manager, &BluetoothLEManager::connectDevice);

    // Connection is established: enable measurement requests
    //
    connect(&m_manager, &BluetoothLEManager::canMeasure,
            this,[this](){
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
        ui->measureButton->setEnabled(true);
        ui->saveButton->setEnabled(false);
    });

    // Disconnect from device
    //
    connect(ui->disconnectButton, &QPushButton::clicked,
            &m_manager, &BluetoothLEManager::disconnectDevice);

    // Request a measurement from the device
    //
    connect(ui->measureButton, &QPushButton::clicked,
          &m_manager, &BluetoothLEManager::measure);

    // Update the UI with any data
    //
    connect(&m_manager, &BluetoothLEManager::dataChanged,
            this,[this](){
        m_manager.buildModel(&m_model);

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
    connect(&m_manager, &BluetoothLEManager::canWrite,
            this,[this](){
       ui->saveButton->setEnabled(true);
    });

    // Write test data to output
    //
    connect(ui->saveButton, &QPushButton::clicked,
      this, &MainWindow::writeOutput);

    // Close the application
    //
    connect(ui->closeButton, &QPushButton::clicked,
            this, &MainWindow::close);

    // Read inputs, such as interview barcode
    //
    readInput();
}

void MainWindow::run()
{
    m_manager.setVerbose(m_verbose);
    m_manager.setRunMode(m_mode);

    // Read the .ini file for cached local and peripheral device addresses
    //
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager.getGroup() + ".json"), JsonSettings::JsonFormat);
    m_manager.loadSettings(settings);

    if(!m_manager.lowEnergyEnabled())
    {
      QMessageBox msgBox;
      msgBox.setText(tr("The host operating system does not support bluetooth "
                        "low energy discovery."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();

      // TODO: return error code
      //
      close();
    }
    if(!m_manager.localDeviceEnabled())
    {
      QMessageBox msgBox;
      msgBox.setText(tr("The host operating system has no local bluetooth "
                          "low energy adapter."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();

      if(m_verbose)
          qDebug() << "failed to find a local adapter";

      // TODO: return error code
      //
      close();
    }

    // Pass the input to the manager for verification
    //
    m_manager.setInputData(m_inputData);

    m_manager.start();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(m_verbose)
        qDebug() << "close event called";
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager.getGroup() + ".json"), JsonSettings::JsonFormat);
    m_manager.saveSettings(&settings);
    m_manager.finish();
    event->accept();
}

void MainWindow::readInput()
{
    // TODO: if the run mode is not debug, an input file name is mandatory, throw an error
    //
    if(m_inputFileName.isEmpty())
    {
        if(Constants::RunMode::modeSimulate == m_mode)
        {
            m_inputData["barcode"] = Constants::DefaultBarcode;
        }
        else
        {
            qDebug() << "ERROR: no input json file";
        }
        return;
    }
    QFileInfo info(m_inputFileName);
    if(info.exists())
    {
      QFile file;
      file.setFileName(m_inputFileName);
      file.open(QIODevice::ReadOnly | QIODevice::Text);
      QString val = file.readAll();
      file.close();

      QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
      m_inputData = jsonDoc.object().toVariantMap();

      if(m_inputData.contains("barcode"))
          ui->barcodeWidget->setBarcode(m_inputData["barcode"].toString());
    }
    else
    {
      if(m_verbose)
        qDebug() << m_inputFileName << " file does not exist";
    }
}

void MainWindow::writeOutput()
{
    if(m_verbose)
        qDebug() << "begin write process ... ";

    QJsonObject jsonObj = m_manager.toJsonObject();

    QString barcode = ui->barcodeWidget->barcode();
    jsonObj.insert("verification_barcode",QJsonValue(barcode));

    if(m_verbose)
        qDebug() << "determine file output name ... ";

    QString fileName;

    // Use the output filename if it has a valid path
    // If the path is invalid, use the directory where the application exe resides
    // If the output filename is empty default output .json file is of the form
    // <participant ID>_<now>_<devicename>.json
    //
    bool constructDefault = false;

    // TODO: if the run mode is not debug, an output file name is mandatory, throw an error
    //
    if(m_outputFileName.isEmpty())
        constructDefault = true;
    else
    {
      QFileInfo info(m_outputFileName);
      QDir dir = info.absoluteDir();
      if(dir.exists())
        fileName = m_outputFileName;
      else
        constructDefault = true;
    }
    if(constructDefault)
    {
        QDir dir = QCoreApplication::applicationDirPath();
        if(m_outputFileName.isEmpty())
        {
          QStringList list;
          list
            << m_manager.getInputDataValue("barcode").toString()
            << QDate().currentDate().toString("yyyyMMdd")
            << m_manager.getGroup()
            << "test.json";
          fileName = dir.filePath( list.join("_") );
        }
        else
          fileName = dir.filePath( m_outputFileName );
    }

    QFile saveFile( fileName );
    saveFile.open(QIODevice::WriteOnly);
    saveFile.write(QJsonDocument(jsonObj).toJson());

    if(m_verbose)
        qDebug() << "wrote to file " << fileName;

   ui->statusBar->showMessage("Temperature data recorded.  Close when ready.");
}
