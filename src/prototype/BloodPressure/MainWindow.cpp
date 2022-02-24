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
{
    ui->setupUi(this);

    // allocate 2 columns x 8 rows of hearing measurement items
    //

    for (int row = 0; row < 8; row++)
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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
    setupConnections();
    initializeArmBandDropDowns();
    initializeButtonState();

    m_manager.setVerbose(m_verbose);
    m_manager.setMode(m_mode);

    readInput();

    populateBarcodeDisplay();

    // read the path to C:\Users\clsa\Documents\CDTT-2018-07-22\CDTTstereo.jar
    //
    QDir dir = QCoreApplication::applicationDirPath();
    qDebug() << "Dir: " << dir;
    QSettings settings(dir.filePath(m_manager.getGroup() + ".ini"), QSettings::IniFormat);
    m_manager.loadSettings(settings);
    // Must load settings before initializing connection ids ui
    initializeConnectionIdsUi();

    // have the manager build the inputs from the input json file
    m_manager.setInputData(m_inputData);
}

void MainWindow::setupConnections()
{
    // bpm connected successfully
    //
    connect(&m_manager, &BloodPressureManager::canMeasure,
        this, [this]() {
            ui->measureButton->setEnabled(true);
            ui->saveButton->setEnabled(false);
            ui->connectButton->setEnabled(false);
            ui->statusBar->showMessage("Connected: Ready to measure blood pressure");
        });

    // Connect to the device (bpm)
    //
    connect(ui->connectButton, &QPushButton::clicked,
        this, [this]() {
            if (m_manager.armInformationSet()) {
                m_manager.connectToBpm();
                // Remove blank enty from selection, so that user cannot change answer to blank
                // The user will still be able to change there selection if they make a mistake
                // But they will be foreced to chose a valid option
                QComboBox* armBandSizeCB = ui->armBandSizeComboBox;
                for (int i = 0; i < armBandSizeCB->count(); i++) {
                    if ("" == armBandSizeCB->itemText(i)) {
                        armBandSizeCB->removeItem(i);
                    }
                }
                QComboBox* armUsedCB = ui->armComboBox;
                for (int i = 0; i < armUsedCB->count(); i++) {
                    if ("" == armUsedCB->itemText(i)) {
                        armUsedCB->removeItem(i);
                    }
                }
            }
            else {
                // this warning message should never be reached. The user should not have 
                // the option to connect until choosing arm band size and arm used.
                // It is here as an added precaution incase the user somehow is able to 
                // click connect before intended
                QMessageBox::warning(
                    this, QApplication::applicationName(),
                    tr("Please select arm band size and arm used before trying to connect"));
            }
        });

    // Request a measurement from the device (bpm)
    //
    connect(ui->measureButton, &QPushButton::clicked,
        this, [this]() {
            ui->statusBar->showMessage("Measuring blood pressure");
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
            ui->statusBar->showMessage("Done Measuring: Blood pressure data ready to be saved");
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
            if (m_manager.armInformationSet() && m_manager.connectionInfoSet()) {
                ui->connectButton->setEnabled(true);
            }
        });
    // Update arm used selected by user
    //
    connect(ui->armComboBox, &QComboBox::currentTextChanged,
        this, [this](const QString& arm) {
            m_manager.setArm(arm);
            if (m_manager.armInformationSet() && m_manager.connectionInfoSet()) {
                ui->connectButton->setEnabled(true);
            }
        });

    // TODO figure out how manager can create its own signals
    connect(&m_manager.m_bpm, &BPM200::connectionStatusReady,
        this, &MainWindow::bpmDisconnected);
    connect(&m_manager.m_bpm, &BPM200::sendError,
        this, [this](const QString& error) {
            ui->statusBar->showMessage("ERROR: Blood Pressure device error.");
            m_manager.finish();
            if (error.isEmpty()) {
                QMessageBox::warning(
                    this, QApplication::applicationName(),
                    tr("ERROR: Blood Pressure device error"));
            }
            else {
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
            m_manager.m_bpm.setConnectionInfo(pid);
            ui->currentPidLabel->setText(QString("Current pid: %1").arg(pid));
            if (m_manager.connectionInfoSet()) {
                ui->selectButton->setEnabled(false);
                ui->refreshButton->setEnabled(false);
                if (m_manager.armInformationSet()) {
                    ui->connectButton->setEnabled(true);
                }
            }
        });

    connect(ui->refreshButton, &QPushButton::clicked,
        this, [this]() {
            updatePossiblePidOptions();
        });

    // Setup Connections for manager
    m_manager.setupConnections();
}

void MainWindow::initializeButtonState()
{
    // disable all buttons by default
    //
    for (auto&& x : this->findChildren<QPushButton*>())
        x->setEnabled(false);

    // Close the application
    //
    ui->closeButton->setEnabled(true);

    // Set the selected arm band size if it has already been selected
    // NOTE: It may have already been set if there is an error while measuring
    QString selectedArmBandSize = ui->armBandSizeComboBox->currentText();
    if ("" != selectedArmBandSize) {
        m_manager.setArmBandSize(selectedArmBandSize);
    }

    // Set the selected arm used if it has already been selected
    // NOTE: It may have already been set if there is an error while measuring
    QString selectedArmUsed = ui->armComboBox->currentText();
    if ("" != selectedArmUsed) {
        m_manager.setArm(selectedArmUsed);
    }

    // Enable the connect button if the arm information has been set
    // NOTE: It may have already been set if there is an error while measuring
    if (m_manager.armInformationSet()) {
        ui->connectButton->setEnabled(true);
    }
}

void MainWindow::initializeArmBandDropDowns()
{
    // The qcombobox automatically selects the first item in the list. So a blank entry is
    // added to display that an option has not been picked yet. The blank entry is treated 
    // as not being a valid option by the rest of this code. If there was no blank entry, then 
    // another option would be set by default and users may forget to change the default option
    ui->armBandSizeComboBox->addItem("");
    ui->armBandSizeComboBox->addItem("Small");
    ui->armBandSizeComboBox->addItem("Medium");
    ui->armBandSizeComboBox->addItem("Large");
    ui->armBandSizeComboBox->addItem("X-Large");

    ui->armComboBox->addItem("");
    ui->armComboBox->addItem("Left");
    ui->armComboBox->addItem("Right");
}

void MainWindow::initializeConnectionIdsUi()
{
    int pid = m_manager.getPid();
    int vid = m_manager.getVid();
    ui->currentPidLabel->setText(QString("Current pid: %1").arg(pid));
    ui->vidLabel->setText(QString("Vid: %1").arg(vid));
    if (false == m_manager.connectionInfoSet()) {
        ui->selectButton->setEnabled(true);
        ui->refreshButton->setEnabled(true);
        updatePossiblePidOptions();

        QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the pid of the blood pressure monitor from the drop down and then click select"));
    }
}

void MainWindow::populateBarcodeDisplay()
{
    // Populate barcode display
    //
    if (m_inputData.contains("barcode") && m_inputData["barcode"].isValid())
        ui->barcodeLineEdit->setText(m_inputData["barcode"].toString());
    else
        ui->barcodeLineEdit->setText("00000000"); // dummy
}

void MainWindow::updatePossiblePidOptions()
{
    // remove current options
    ui->newPidComboBox->clear();
    /*int comboBoxOptionCount = ui->newPidComboBox->count();
    for (int i = 0; i < comboBoxOptionCount; i++) {
        ui->newPidComboBox->removeItem(comboBoxOptionCount - i - 1);
    }*/
    // add new options based on current connected usb devices with bpm vid
    QList<int> possiblePids = m_manager.m_bpm.findAllPids();
    for each (int pid in possiblePids) {
        ui->newPidComboBox->addItem(QString("%1").arg(pid));
    }
}

void MainWindow::run()
{
    m_manager.start();
}

void MainWindow::bpmDisconnected(const bool &connected)
{
    if (connected == false) {
        initializeButtonState();
        ui->statusBar->showMessage("ERROR: Cannot connect to blood pressure monitor");
        QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("ERROR: Cannot connect to blood pressure monitor. Please ensure that the blood pressure monitor is plugged in, turned on and connected to the computer"));
    }
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
        qDebug() << "no input file";
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
        qDebug() << val;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        QMapIterator<QString, QVariant> it(m_inputData);
        QList<QString> keys = jsonObj.keys();
        for (int i = 0; i < keys.size(); i++)
        {
            QJsonValue v = jsonObj.value(keys[i]);
            // TODO: error report all missing expected key values
            //
            if (!v.isUndefined())
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
    if (m_verbose)
        qDebug() << "begin write process ... ";

    QJsonObject jsonObj = m_manager.toJsonObject();

    QString barcode = ui->barcodeLineEdit->text().simplified().remove(" ");
    jsonObj.insert("barcode", QJsonValue(barcode));

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
                << barcode
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

    ui->statusBar->showMessage("Blood Pressure data recorded.  Close when ready.");
}
