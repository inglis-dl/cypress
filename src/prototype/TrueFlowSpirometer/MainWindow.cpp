#include "MainWindow.h"
#include "ui_MainWindow.h"

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
#include <QTimeLine>

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
  m_manager.start();
}

void MainWindow::initializeModel()
{
    // allocate 2 columns x 8 rows of hearing measurement items
    //
    for (int col = 0; col < 2; col++)
    {
        for (int row = 0; row < 8; row++)
        {
            QStandardItem* item = new QStandardItem();
            m_model.setItem(row, col, item);
        }
    }
    m_model.setHeaderData(0, Qt::Horizontal, "Left Test Results", Qt::DisplayRole);
    m_model.setHeaderData(1, Qt::Horizontal, "Right Test Results", Qt::DisplayRole);
    ui->testdataTableView->setModel(&m_model);

    ui->testdataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->testdataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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
  connect(&m_manager,&ManagerBase::message,
          ui->statusBar, &QStatusBar::showMessage, Qt::DirectConnection);

  if (Constants::RunMode::modeSimulate == m_mode)
  {
      ui->barcodeWidget->setBarcode(Constants::DefaultBarcode);
  }

  connect(ui->barcodeWidget, &BarcodeWidget::validated,
      this, [this](const bool& valid)
      {
          if (valid)
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
    connect(&m_manager, &TrueFlowSpirometerManager::canMeasure,
        this, [this]() {
            ui->measureButton->setEnabled(true);
            ui->saveButton->setEnabled(false);
        });

    // Request a measurement from the device (run CCB.exe)
    //
    connect(ui->measureButton, &QPushButton::clicked,
        &m_manager, &TrueFlowSpirometerManager::measure);

    // Update the UI with any data
    //
    connect(&m_manager, &TrueFlowSpirometerManager::dataChanged,
        this, [this]() {
            auto h = ui->testdataTableView->horizontalHeader();
            h->setSectionResizeMode(QHeaderView::Fixed);

            m_manager.buildModel(&m_model);

            QSize ts_pre = ui->testdataTableView->size();
            h->resizeSections(QHeaderView::ResizeToContents);
            ui->testdataTableView->setColumnWidth(0, h->sectionSize(0));
            ui->testdataTableView->setColumnWidth(1, h->sectionSize(1));
            ui->testdataTableView->resize(
                h->sectionSize(0) + h->sectionSize(1) +
                ui->testdataTableView->autoScrollMargin(),
                8 * ui->testdataTableView->rowHeight(0) + 1 +
                h->height());
            QSize ts_post = ui->testdataTableView->size();
            int dx = ts_post.width() - ts_pre.width();
            int dy = ts_post.height() - ts_pre.height();
            this->resize(this->width() + dx, this->height() + dy);
        });

    // All measurements received: enable write test results
    //
    connect(&m_manager, &TrueFlowSpirometerManager::canWrite,
        this, [this]() {
            ui->saveButton->setEnabled(true);
        });

    // Close the application
    //
    connect(ui->closeButton, &QPushButton::clicked,
        this, &MainWindow::close);

    // Write test data to output
    //
    connect(ui->saveButton, &QPushButton::clicked,
        this, &MainWindow::writeOutput);

    connect(&m_manager, &TrueFlowSpirometerManager::canSelectRunnable,
        this, [this]() {
            bool openTransferDirButtonEnabled = ui->openTransferDirButton->isEnabled();
            for (auto&& x : this->findChildren<QPushButton*>())
                x->setEnabled(false);
            ui->closeButton->setEnabled(true);
            ui->openButton->setEnabled(true);
            ui->openTransferDirButton->setEnabled(openTransferDirButtonEnabled);
            static bool warn = true;
            if (warn)
            {
                QMessageBox::warning(
                    this, QApplication::applicationName(),
                    tr("Select the exe by clicking Open and browsing to the "
                        "required executable (EasyWarePro.exe) and selecting the file.  If the executable "
                        "is valid click the Run button to start the test otherwise check the installation."));
                warn = false;
            }
        });

    connect(ui->openButton, &QPushButton::clicked,
        this, [this]() {
            QString fileName =
                QFileDialog::getOpenFileName(
                    this, tr("Open File"),
                    QCoreApplication::applicationDirPath(),
                    tr("Applications (*.jar, *)"));

            m_manager.selectRunnable(fileName);
        });

    connect(&m_manager, &TrueFlowSpirometerManager::canSelectEmrTransferDir,
        this, [this]() {
            bool openButtonEnabled = ui->openButton->isEnabled();
            for (auto&& x : this->findChildren<QPushButton*>())
                x->setEnabled(false);
            ui->closeButton->setEnabled(true);
            ui->openTransferDirButton->setEnabled(true);
            ui->openButton->setEnabled(openButtonEnabled);
            static bool warn = true;
            if (warn)
            {
                QMessageBox::warning(
                    this, QApplication::applicationName(),
                    tr("Select the emr transfer directory by clicking Open and browsing to the "
                        "required transfer directory (likely C:/ProgramData/ndd/Easy on-PC) and selecting the it. "
                        "Click the Run button to start the test otherwise check the installation."));
                warn = false;
            }
        });

    connect(ui->openTransferDirButton, &QPushButton::clicked,
        this, [this]() {
            QString fileName =
                QFileDialog::getExistingDirectory(
                    this, tr("Open Directory"),
                    QCoreApplication::applicationDirPath());

            m_manager.selectEmrTransferDir(fileName);
        });

    // Read inputs required to launch frax test
    // In simulate mode the barcode is always populated with a default of "00000000"
    //
    readInput();
}

void MainWindow::run()
{
    m_manager.setVerbose(m_verbose);
    m_manager.setRunMode(m_mode);

     // read the path to
    //
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager.getGroup() + ".ini"), QSettings::IniFormat);
    m_manager.loadSettings(settings);

    // Pass the input to the manager for verification
    //
    m_manager.setInputData(m_inputData);

    m_manager.start();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_verbose)
        qDebug() << "close event called";

    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager.getGroup() + ".ini"), QSettings::IniFormat);
    m_manager.saveSettings(&settings);
    m_manager.finish();
    event->accept();
}

void MainWindow::readInput()
{
    // TODO: if the run mode is not debug, an input file name is mandatory, throw an error
    //
    if (m_inputFileName.isEmpty())
    {
        if (Constants::RunMode::modeSimulate == m_mode)
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
    if (info.exists())
    {
        QFile file;
        file.setFileName(m_inputFileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString val = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        foreach(auto key, jsonObj.keys())
        {
            QJsonValue v = jsonObj.value(key);
            // TODO: error report all missing expected key values
            //
            if (!v.isUndefined())
            {
                m_inputData[key] = v.toVariant();
                qDebug() << key << v.toVariant();
            }
        }
        if (m_inputData.contains("barcode"))
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
    if (m_verbose)
        qDebug() << "begin write process ... ";

    QJsonObject jsonObj = m_manager.toJsonObject();

    QString barcode = ui->barcodeWidget->barcode();
    jsonObj.insert("verification_barcode", QJsonValue(barcode));

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
    if (m_outputFileName.isEmpty())
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
    if (constructDefault)
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

    ui->statusBar->showMessage("Weigh scale data recorded.  Close when ready.");
}
