#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../../auxiliary/JsonSettings.h"

#include <QCloseEvent>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
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

  // Read inputs required to launch Easy On-PC
  // In simulate mode the barcode is always populated with a default of "00000000"
  //
  readInput();

  m_manager.start();
}

void MainWindow::initializeModel()
{
    m_manager.initializeModel();
    ui->measureWidget->initialize(m_manager.getModel());
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
      if("Close" != button->text())
        button->setEnabled(false);

      // disable enter key press event passing onto auto focus buttons
      //
      button->setDefault(false);
      button->setAutoDefault(false);
  }

  // Relay messages from the manager to the status bar
  //
  connect(&m_manager,&ManagerBase::message,
          ui->statusBar, &QStatusBar::showMessage, Qt::DirectConnection);

  if(Constants::RunMode::modeSimulate == m_mode)
  {
      ui->barcodeWidget->setBarcode(Constants::DefaultBarcode);
  }

  connect(ui->barcodeWidget, &BarcodeWidget::validated,
      this, [this](const bool& valid)
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

    // Available to start measuring
    //
    connect(&m_manager, &SpirometerManager::canMeasure,
            ui->measureWidget, &MeasureWidget::enableMeasure);

    // Request a measurement from the device
    //
    connect(ui->measureWidget, &MeasureWidget::measure,
        &m_manager, &SpirometerManager::measure);

    // Update the UI with any data
    //
    connect(&m_manager, &SpirometerManager::dataChanged,
        ui->measureWidget, &MeasureWidget::updateModelView);

    // All measurements received: enable write test results
    //
    connect(&m_manager, &SpirometerManager::canWrite,
        ui->measureWidget, &MeasureWidget::enableWriteToFile);

    // Write test data to output
    //
    connect(ui->measureWidget, &MeasureWidget::writeToFile,
        this, &MainWindow::writeOutput);

    // Provide file / path selection dialog as needed
    //
    connect(ui->openButton, &QPushButton::clicked,
        &m_manager, &SpirometerManager::select);

    connect(&m_manager, &SpirometerManager::canSelectRunnable,
        this, [this]() {
        foreach(auto button, this->findChildren<QPushButton *>())
        {
          if("Close" != button->text())
            button->setEnabled(false);
        }
        ui->openButton->setEnabled(true);
        static bool warn = true;
        if(warn)
        {
          QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the exe by clicking Open and browsing to the "
               "required executable (EasyWarePro.exe) and selecting the file.  If the executable "
               "is valid click the Run button to start the test otherwise check the installation."));
          warn = false;
        }
      });

    connect(&m_manager, &SpirometerManager::canSelectDataPath,
        this, [this]() {
        foreach(auto button, this->findChildren<QPushButton *>())
        {
          if("Close" != button->text())
            button->setEnabled(false);
        }
        ui->openButton->setEnabled(true);
        static bool warn = true;
        if(warn)
        {
          QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the EMR transfer directory by clicking Open and browsing to the "
               "required directory (likely C:/ProgramData/ndd/Easy on-PC) and selecting the it. "
               "Click the Run button to start the test otherwise check the installation."));
          warn = false;
        }
      });

    // Close the application
    //
    connect(ui->measureWidget, &MeasureWidget::closeApplication,
        this, &MainWindow::close);
}

void MainWindow::run()
{
    m_manager.setVerbose(m_verbose);
    m_manager.setRunMode(m_mode);

     // read the path to
    //
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager.getGroup() + ".json"), JsonSettings::JsonFormat);
    m_manager.loadSettings(settings);

    // Pass the input to the manager for verification
    //
    m_manager.setInputData(m_inputData);

    m_manager.start();
}

void MainWindow::closeEvent(QCloseEvent* event)
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
            if (m_verbose)
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
        if (m_verbose)
            qDebug() << m_inputFileName << " file does not exist";
    }
}

void MainWindow::writeOutput()
{
    if(m_verbose)
        qDebug() << "begin write process ... ";

    QJsonObject jsonObj = m_manager.toJsonObject();

    QString barcode = ui->barcodeWidget->barcode();
    jsonObj.insert("verification_barcode", QJsonValue(barcode));

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
        if (dir.exists())
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
            fileName = dir.filePath(list.join("_"));
        }
        else
            fileName = dir.filePath(m_outputFileName);
    }

    QFile saveFile(fileName);
    saveFile.open(QIODevice::WriteOnly);
    saveFile.write(QJsonDocument(jsonObj).toJson());

    if(m_verbose)
        qDebug() << "wrote to file " << fileName;

    ui->statusBar->showMessage("Spirometer data recorded.  Close when ready.");
}
