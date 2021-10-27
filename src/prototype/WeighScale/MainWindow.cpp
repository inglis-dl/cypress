#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCloseEvent>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_verbose(false)
    , m_manager(this)
{
    ui->setupUi(this);

    for(int row=0;row<2;row++)
    {
      QStandardItem* item = new QStandardItem();
      m_model.setItem(row,0,item);
    }
    m_model.setHeaderData(0,Qt::Horizontal,"Weight Tests",Qt::DisplayRole);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
  m_manager.setVerbose(m_verbose);

  readInput();

  if(m_inputData.contains("Barcode") && m_inputData["Barcode"].isValid())
     ui->barcodeLineEdit->setText(m_inputData["Barcode"].toString());
  else
     ui->barcodeLineEdit->setText("00000000"); // dummy

  // Read the .ini file for cached local and peripheral device addresses
  //
  QDir dir = QCoreApplication::applicationDirPath();
  QSettings settings(dir.filePath("weighscale.ini"), QSettings::IniFormat);
  m_manager.loadSettings(settings);

  // Save button to store measurement and device info to .json
  //
  ui->saveButton->setEnabled(false);

  // Zero the scale
  //
  ui->zeroButton->setEnabled(false);

  // Read the weight measurement off the scale
  //
  ui->measureButton->setEnabled(false);

  // Connect to the device
  //
  ui->connectButton->setEnabled(false);

  // Disconnect from the device
  //
  ui->disconnectButton->setEnabled(false);

  // Scan for devices
  //
  connect(&m_manager, &WeighScaleManager::scanningDevices,
          this,[this]()
    {
      ui->deviceComboBox->clear();
      ui->statusBar->showMessage("Discovering serial ports...");
    }
  );

  // Update the drop down list as devices are discovered during scanning
  //
  connect(&m_manager, &WeighScaleManager::deviceDiscovered,
          this, &MainWindow::updateDeviceList);

  // Prompt user to select a device from the drop down list when previously
  // cached device information in the ini file is unavailable or invalid
  //
  connect(&m_manager, &WeighScaleManager::canSelectDevice,
          this,[this](){
      ui->statusBar->showMessage("Ready to select...");
      QMessageBox msgBox;
      msgBox.setText(tr("Select the port from the list.  If the device "
        "is not in the list, quit the application and check that the port is "
        "working and connect the audiometer to it before running this application."));
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);
      msgBox.setButtonText(QMessageBox::Abort,tr("Quit"));
      connect(msgBox.button(QMessageBox::Abort),&QPushButton::clicked,this,&MainWindow::close);
      msgBox.exec();
  });

  // Select a device (serial port) from drop down list
  //
  connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,[this]()
    {
      if(m_verbose)
          qDebug() << "device selected from list " <<  ui->deviceComboBox->currentText();
      m_manager.selectDevice(ui->deviceComboBox->currentText());
    }
  );

  // Ready to connect device
  //
  connect(&m_manager, &WeighScaleManager::canConnectDevice,
          this,[this](){
      qDebug() << "ready to connect";
      ui->statusBar->showMessage("Ready to connect...");
      ui->connectButton->setEnabled(true);
      ui->zeroButton->setEnabled(false);
      ui->disconnectButton->setEnabled(false);
      ui->measureButton->setEnabled(false);
      ui->saveButton->setEnabled(false);
  });

  // Connect to device
  //
  connect(ui->connectButton, &QPushButton::clicked,
          &m_manager, &WeighScaleManager::connectDevice);

  // Connection is established: enable measurement requests
  //
  connect(&m_manager, &WeighScaleManager::canMeasure,
          this,[this](){
      qDebug() << "ready to measure";
      ui->statusBar->showMessage("Ready to measure...");
      ui->connectButton->setEnabled(false);
      ui->disconnectButton->setEnabled(true);
      ui->zeroButton->setEnabled(true);
      ui->measureButton->setEnabled(true);
  });

  // Disconnect from device
  //
  connect(ui->disconnectButton, &QPushButton::clicked,
          &m_manager, &WeighScaleManager::disconnectDevice);

  // Zero the scale
  //
  connect(ui->zeroButton, &QPushButton::clicked,
        &m_manager, &WeighScaleManager::zeroDevice);

  // Request a measurement from the device
  //
  connect(ui->measureButton, &QPushButton::clicked,
        &m_manager, &WeighScaleManager::measure);

  // Update the UI with any data
  //
  connect(&m_manager, &WeighScaleManager::dataChanged,
          this,[this](){
      m_manager.buildModel(&m_model);
      ui->testdataTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  });

  // All measurements received: enable write test results
  //
  connect(&m_manager, &WeighScaleManager::canWrite,
          this,[this](){
      qDebug() << "ready to measure";
      ui->statusBar->showMessage("Ready to measure...");
      ui->measureButton->setEnabled(true);
  });

  // Write test data to output
  //
  connect(ui->saveButton, &QPushButton::clicked,
    this, &MainWindow::writeOutput);

  // Close the application
  //
  connect(ui->closeButton, &QPushButton::clicked,
          this, &MainWindow::close);

  emit m_manager.dataChanged();
}

void MainWindow::updateDeviceList(const QString &label)
{
    // Add the device to the list
    //
    int index = ui->deviceComboBox->findText(label);
    if(-1 == index)
    {
        ui->deviceComboBox->addItem(label);
    }
}

void MainWindow::run()
{
    m_manager.scanDevices();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(m_verbose)
        qDebug() << "close event called";
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath("weighscale.ini"), QSettings::IniFormat);
    m_manager.saveSettings(&settings);

    event->accept();
}

void MainWindow::readInput()
{
    // TODO: if the run mode is not debug, an input file name is mandatory, throw an error
    //
    if(m_inputFileName.isEmpty())
    {
        qDebug() << "no input file";
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
      qDebug() << val;

      QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
      QJsonObject jsonObj = jsonDoc.object();
      QMapIterator<QString,QVariant> it(m_inputData);
      QList<QString> keys = jsonObj.keys();
      for(int i=0;i<keys.size();i++)
      {
          QJsonValue v = jsonObj.value(keys[i]);
          // TODO: error report all missing expected key values
          //
          if(!v.isUndefined())
          {
              m_inputData[keys[i]] = v.toVariant();
              qDebug() << keys[i] << v.toVariant();
          }
      }
    }
    else
        qDebug() << m_inputFileName << " file does not exist";
}

void MainWindow::writeOutput()
{
   if(m_verbose)
       qDebug() << "begin write process ... ";

   QJsonObject jsonObj = m_manager.toJsonObject();

   qDebug() << "received json measurement data";

   QString barcode = ui->barcodeLineEdit->text().simplified().remove(" ");
   jsonObj.insert("barcode",QJsonValue(barcode));

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
         list << barcode;
         list << QDate().currentDate().toString("yyyyMMdd");
         list << "weighscale.json";
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

   ui->statusBar->showMessage("Weigh scale data recorded.  Close when ready.");
}
