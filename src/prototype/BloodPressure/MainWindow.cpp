#include "MainWindow.h"

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
    initializeButtonState();
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

    QStringList armList = (QStringList()<<""<<"Left"<<"Right");
    ui->armComboBox->addItems(armList);

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

    // bpm connected successfully
    //
    connect(&m_manager, &BloodPressureManager::canMeasure,
        this, [this]() {
            ui->measureButton->setEnabled(true);
            ui->saveButton->setEnabled(false);
            ui->connectButton->setEnabled(false);
        });

    // Ready to connect device
    //
    connect(&m_manager, &BloodPressureManager::canConnectDevice,
            this,[this]() {
        ui->connectButton->setEnabled(true);
        ui->measureButton->setEnabled(false);
        ui->saveButton->setEnabled(false);
    });

    // Connect to the device (bpm)
    //
    connect(ui->connectButton, &QPushButton::clicked,
        this, [this]() {
      m_manager.connectToBpm();

      // Remove blank enty from selection, so that user cannot change answer to blank
      // The user will still be able to change their selection if they make a mistake
      // But they will be forced to chose a valid option
      //
      ui->armBandSizeComboBox->removeItem(ui->armBandSizeComboBox->findText(""));
      ui->armComboBox->removeItem(ui->armComboBox->findText(""));
    });

    // Request a measurement from the device (bpm)
    //
    connect(ui->measureButton, &QPushButton::clicked,
        this, [this]() {
            ui->measureButton->setEnabled(false);
            m_manager.measure();
        });

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

    // Update arm band size selected by user
    //
    connect(ui->armBandSizeComboBox, &QComboBox::currentTextChanged, 
        this, [this](const QString &size) {
            m_manager.setArmBandSize(size);
        });

    // Update arm used selected by user
    //
    connect(ui->armComboBox, &QComboBox::currentTextChanged,
        this, [this](const QString& arm) {
            m_manager.setArm(arm);
        });

    // TODO figure out how manager can create its own signals
    connect(&m_manager.m_bpm, &BPM200::connectionStatusReady,
            this, [this](const bool& connected) {
            if(!connected)
            {
                initializeButtonState();
                ui->statusBar->showMessage("ERROR: Cannot connect to blood pressure monitor");
                QMessageBox::warning(
                    this, QApplication::applicationName(),
                    tr("ERROR: Cannot connect to blood pressure monitor. Please ensure that the blood pressure monitor is plugged in, turned on and connected to the computer"));
            }
         });

    connect(&m_manager.m_bpm, &BPM200::sendError,
        this, [this](const QString& error) {
            ui->statusBar->showMessage("ERROR: Blood Pressure device error.");
            m_manager.finish();
            if(error.isEmpty())
            {
                QMessageBox::warning(
                    this, QApplication::applicationName(),
                    tr("ERROR: Blood Pressure device error"));
            }
            else
            {
                QMessageBox::warning(
                    this, QApplication::applicationName(),
                    tr(error.toLocal8Bit().data()));
            }
            initializeButtonState();
        });

    // Select pid of the blood pressure monitor to connect to
    //
    connect(ui->selectButton, &QPushButton::clicked,
        this, [this]() {
            ui->statusBar->showMessage("Pid selected");
            int pid = ui->newPidComboBox->currentText().toInt();
            m_manager.setDevice(pid);
            ui->currentPidLabel->setText(QString("Current pid: %1").arg(pid));
        });

    connect(ui->refreshButton, &QPushButton::clicked,
        this, [this]() {
        // remove current options
        ui->newPidComboBox->clear();
        // add new options based on current connected usb devices with bpm vid
        QList<int> possiblePids = m_manager.m_bpm.findAllPids();
        foreach(int pid, possiblePids)
        {
          ui->newPidComboBox->addItem(QString("%1").arg(pid));
        }
     });

    // Read inputs, such as interview barcode
    //
    readInput();
}

void MainWindow::initializeButtonState()
{
    // Set the selected arm band size if it has already been selected
    // NOTE: It may have already been set if there is an error while measuring
    //
    QString selectedArmBandSize = ui->armBandSizeComboBox->currentText();
    if("" != selectedArmBandSize)
    {
        m_manager.setArmBandSize(selectedArmBandSize);
    }

    // Set the selected arm used if it has already been selected
    // NOTE: It may have already been set if there is an error while measuring
    //
    QString selectedArmUsed = ui->armComboBox->currentText();
    if("" != selectedArmUsed)
    {
        m_manager.setArm(selectedArmUsed);
    }
}

void MainWindow::initializeConnectionIdsUi()
{
    int pid = m_manager.getPid();
    int vid = m_manager.getVid();
    ui->currentPidLabel->setText(QString("Current pid: %1").arg(pid));
    ui->vidLabel->setText(QString("Vid: %1").arg(vid));
    if(!m_manager.connectionInfoSet())
    {
        ui->selectButton->setEnabled(true);
        ui->refreshButton->setEnabled(true);
        ui->refreshButton->click();
        QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the pid of the blood pressure monitor from the drop down and then click select"));
    }
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
