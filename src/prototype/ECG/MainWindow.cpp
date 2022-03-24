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
    // allocate 1 columns x n rows of ECG measurement items
    //
    for(int row=0; row<m_manager.getNumberOfModelRows(); row++)
    {
      QStandardItem* item = new QStandardItem();
      m_model.setItem(row, 0, item);
    }

    m_model.setHeaderData(0, Qt::Horizontal, "Electrocardiogram Results ", Qt::DisplayRole);
    ui->testdataTableView->setModel(&m_model);

    ui->testdataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

    connect(&m_manager,&ECGManager::canSelectRunnable,
            this,[this](){
        foreach(auto button, this->findChildren<QPushButton *>())
            button->setEnabled(false);
        ui->closeButton->setEnabled(true);
        ui->openButton->setEnabled(true);
        static bool warn = true;
        if(warn)
        {
            QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the exe by clicking Open and browsing to the "
            "required executable (Cardio.exe) and selecting the file.  If the executable "
            "is valid click the Run button to start the test otherwise check the installation."));
            warn = false;
        }
    });

    connect(ui->openButton, &QPushButton::clicked,
            &m_manager, &ECGManager::select);

    connect(&m_manager,&ECGManager::canSelectWorking,
            this,[this](){
        foreach(auto button, this->findChildren<QPushButton *>())
            button->setEnabled(false);
        ui->closeButton->setEnabled(true);
        ui->openButton->setEnabled(true);
        static bool warn = true;
        if(warn)
        {
            QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the path to the Cardiosoft database directory by clicking Open and browsing to "
            "and selecting the folder (C:/CARDIO).  If the folder "
            "is valid click the Run button to start the test otherwise check the installation."));
            warn = false;
        }
    });

    // Cardio.exe was found and inputs valid
    //
    connect(&m_manager, &ECGManager::canMeasure,
        this, [this]() {
            ui->measureButton->setEnabled(true);
            ui->saveButton->setEnabled(false);
        });

    // Request a measurement from the device (run blackbox.exe)
    //
    connect(ui->measureButton, &QPushButton::clicked,
        &m_manager, &ECGManager::measure);

    // Update the UI with any data
    //
    connect(&m_manager, &ECGManager::dataChanged,
        this, [this]() {
      auto h = ui->testdataTableView->horizontalHeader();
      h->setSectionResizeMode(QHeaderView::Fixed);

      m_manager.buildModel(&m_model);

      QSize ts_pre = ui->testdataTableView->size();
      h->resizeSections(QHeaderView::ResizeToContents);
      ui->testdataTableView->setColumnWidth(0, h->sectionSize(0));
      ui->testdataTableView->resize(
        h->sectionSize(0) + 1,
        12*(ui->testdataTableView->rowHeight(0)) + 1 +
        h->height());
      QSize ts_post = ui->testdataTableView->size();
      int dx = ts_post.width() - ts_pre.width();
      int dy = ts_post.height() - ts_pre.height();
      this->resize(this->width() + dx, this->height() + dy);
    });

    // All measurements received: enable write test results
    //
    connect(&m_manager, &ECGManager::canWrite,
        this, [this]() {
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

    // Read inputs required to launch frax test
    //
    readInput();
}

void MainWindow::run()
{
    m_manager.setVerbose(m_verbose);
    m_manager.setRunMode(m_mode);

    // read the fullspec path to Cardio.exe and the path to C:/CARDIO
    //
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager.getGroup() + ".json"), JsonSettings::JsonFormat);
    m_manager.loadSettings(settings);

    // Pass the input to the manager for verification
    //
    m_manager.setInputData(m_inputData);

    m_manager.start();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_verbose)
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
          if(m_verbose)
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
      m_inputData = jsonDoc.object();

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
    if (m_verbose)
        qDebug() << "begin write process ... ";

    QJsonObject jsonObj = m_manager.toJsonObject();

    QString barcode = ui->barcodeWidget->barcode();
    jsonObj.insert("verification_barcode",QJsonValue(barcode));

    if (m_verbose)
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
        if (m_outputFileName.isEmpty())
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

    if (m_verbose)
        qDebug() << "wrote to file " << fileName;

    ui->statusBar->showMessage("ECG data recorded.  Close when ready.");
}
