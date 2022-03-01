#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QList>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>

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

void MainWindow::initializeModel()
{
    for(int row = 0; row < m_manager.getNumberOfModelRows(); row++)
    {
        QStandardItem* item = new QStandardItem();
        m_model.setItem(row, 0, item);
    }
    m_model.setHeaderData(0, Qt::Horizontal, "Test Results", Qt::DisplayRole);
    ui->testdataTableView->setModel(&m_model);

    ui->testdataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->testdataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->testdataTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->verticalHeader()->hide();
}

void MainWindow::initialize()
{
    initializeModel();
    initializeConnections();
}

void MainWindow::run()
{
    m_manager.setVerbose(m_verbose);
    m_manager.setRunMode(m_mode);

    // Read the .ini file for cached device data
    //
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager.getGroup() + ".ini"), QSettings::IniFormat);
    m_manager.loadSettings(settings);

    // Pass the input to the manager for verification
    //
    m_manager.setInputData(m_inputData);

    m_manager.start();
}

void MainWindow::initializeConnections()
{
    // The qcombobox automatically selects the first item in the list. So a blank entry is
    // added to display that an option has not been picked yet. The blank entry is treated
    // as not being a valid option by the rest of this code. If there was no blank entry, then
    // another option would be set by default and users may forget to change the default option
    //
    QStringList bandList = (QStringList()<<""<<"Small"<<"Medium"<<"Large"<<"X-large");
    ui->armBandSizeComboBox->addItems(bandList);
    ui->armBandSizeComboBox->setCurrentIndex(0);

    QStringList armList = (QStringList()<<""<<"Left"<<"Right");
    ui->armComboBox->addItems(armList);
    ui->armComboBox->setCurrentIndex(0);

    // Update cuff size selected by user
    //
    connect(ui->armBandSizeComboBox, &QComboBox::currentTextChanged,
        this, [this](const QString &size) {
            m_manager.setCuffSize(size);
        });

    // Update arm used selected by user
    //
    connect(ui->armComboBox, &QComboBox::currentTextChanged,
        this, [this](const QString& side) {
            m_manager.setSide(side);
        });

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

    // Scan for devices
    //
    connect(&m_manager, &BloodPressureManager::scanningDevices,
            ui->deviceComboBox, &QComboBox::clear);

    // Update the drop down list as devices are discovered during scanning
    //
    connect(&m_manager, &BloodPressureManager::deviceDiscovered,
            this,[this](const QString &label){
        int index = ui->deviceComboBox->findText(label);
        bool oldState = ui->deviceComboBox->blockSignals(true);
        if(-1 == index)
        {
            ui->deviceComboBox->addItem(label);
        }
        ui->deviceComboBox->blockSignals(oldState);
    });

    connect(&m_manager, &BloodPressureManager::deviceSelected,
            this,[this](const QString &label){
        if(label!=ui->deviceComboBox->currentText())
        {
            ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findText(label));
        }
    });

    // Prompt user to select a device from the drop down list when previously
    // cached device information in the ini file is unavailable or invalid
    //
    connect(&m_manager, &BloodPressureManager::canSelectDevice,
            this,[this](){
        QMessageBox::warning(
          this, QApplication::applicationName(),
          tr("Select the device from the list.  If the device "
          "is not in the list, quit the application and check that the usb port is "
          "working and connect the blood pressure monitor to it before running this application."));
    });

    // Select a device from drop down list
    //
    connect(ui->deviceComboBox, &QComboBox::currentTextChanged,
            &m_manager,&BloodPressureManager::selectDevice);

    // Select a device from drop down list
    //
    connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::activated),
      this,[this](int index){
        m_manager.selectDevice(ui->deviceComboBox->itemText(index));
    });

    // Ready to connect device
    //
    connect(&m_manager, &BloodPressureManager::canConnectDevice,
            this,[this]() {
        ui->connectButton->setEnabled(true);
        ui->disconnectButton->setEnabled(false);
        ui->measureButton->setEnabled(false);
        ui->saveButton->setEnabled(false);
    });

    // Connect to the device (bpm)
    //
    connect(ui->connectButton, &QPushButton::clicked,
        &m_manager, &BloodPressureManager::connectDevice);

    // Connection is established: enable measurement requests
    //
    connect(&m_manager, &BloodPressureManager::canMeasure,
            this,[this](){
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
        ui->measureButton->setEnabled(true);
        ui->saveButton->setEnabled(false);

        // Remove blank enty from selection, so that user cannot change answer to blank
        // The user will still be able to change their selection if they make a mistake
        // But they will be forced to chose a valid option
        //
        ui->armBandSizeComboBox->removeItem(ui->armBandSizeComboBox->findText(""));
        ui->armComboBox->removeItem(ui->armComboBox->findText(""));
    });

    // Disconnect from device
    //
    connect(ui->disconnectButton, &QPushButton::clicked,
            &m_manager, &BloodPressureManager::disconnectDevice);

    // Request a measurement from the device (bpm)
    //
    connect(ui->measureButton, &QPushButton::clicked,
            &m_manager, &BloodPressureManager::measure);

    // Update the UI with any data
    //
    connect(&m_manager, &BloodPressureManager::dataChanged,
        this, [this]() {
            auto h = ui->testdataTableView->horizontalHeader();
            h->setSectionResizeMode(QHeaderView::Fixed);
            m_manager.buildModel(&m_model);
            QSize ts_pre = ui->testdataTableView->size();
            h->resizeSections(QHeaderView::ResizeToContents);
            ui->testdataTableView->setColumnWidth(0, h->sectionSize(0));
            ui->testdataTableView->resize(
                h->sectionSize(0) +
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
    connect(&m_manager, &BloodPressureManager::canWrite,
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

    // Read inputs, such as interview barcode
    //
    readInput();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if(m_verbose)
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
    if(m_inputFileName.isEmpty())
    {
        if(CypressConstants::RunMode::Simulate == m_mode)
        {
            m_inputData["barcode"]="00000000";
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
      QJsonObject jsonObj = jsonDoc.object();
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
      if(m_inputData.contains("barcode"))
          ui->barcodeWidget->setBarcode(m_inputData["barcode"].toString());
    }
    else
        qDebug() << m_inputFileName << " file does not exist";
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

    ui->statusBar->showMessage("Audiometer data recorded.  Close when ready.");
}
