#include "MainWindow.h"

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

MainWindow::~MainWindow()
{
    m_manager.finish();
    delete ui;
}

void MainWindow::initialize()
{
    setupConnections();
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

    // have the manager build the inputs from the input json file
    m_manager.setInputData(m_inputData);

    validateRunnablePresense();
}

void MainWindow::setupConnections()
{
    // Close the application
    //
    connect(ui->closeButton, &QPushButton::clicked,
        this, &MainWindow::close);
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

void MainWindow::validateRunnablePresense()
{
    // validate the presence of runnable and enable
    // file selection as required
    //
    /*QString runnableName = m_manager.getRunnableName();
    if (!m_manager.isDefined(runnableName))
    {
        ui->openButton->setEnabled(true);
        QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the jar by clicking Open and browsing to the "
                "required jar (CDTTstereo.jar) and selecting the file.  If the jar "
                "is valid click the Run button to start the test otherwise check the installation."));
    }*/
}

void MainWindow::run()
{
    m_manager.start();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_verbose)
        qDebug() << "close event called";

    /*QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager.getGroup() + ".ini"), QSettings::IniFormat);
    m_manager.saveSettings(&settings);
    m_manager.finish();*/
    event->accept();
}

void MainWindow::readInput()
{
    //// TODO: if the run mode is not debug, an input file name is mandatory, throw an error
    ////
    //if (m_inputFileName.isEmpty())
    //{
    //    qDebug() << "no input file";
    //    return;
    //}
    //QFileInfo info(m_inputFileName);
    //if (info.exists())
    //{
    //    QFile file;
    //    file.setFileName(m_inputFileName);
    //    file.open(QIODevice::ReadOnly | QIODevice::Text);
    //    QString val = file.readAll();
    //    file.close();
    //    qDebug() << val;

    //    QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
    //    QJsonObject jsonObj = jsonDoc.object();
    //    QMapIterator<QString, QVariant> it(m_inputData);
    //    QList<QString> keys = jsonObj.keys();
    //    for (int i = 0; i < keys.size(); i++)
    //    {
    //        QJsonValue v = jsonObj.value(keys[i]);
    //        // TODO: error report all missing expected key values
    //        //
    //        if (!v.isUndefined())
    //        {
    //            m_inputData[keys[i]] = v.toVariant();
    //            qDebug() << keys[i] << v.toVariant();
    //        }
    //    }
    //}
    //else
    //    qDebug() << m_inputFileName << " file does not exist";
}

void MainWindow::writeOutput()
{
    //if (m_verbose)
    //    qDebug() << "begin write process ... ";

    //QJsonObject jsonObj = m_manager.toJsonObject();

    //QString barcode = ui->barcodeLineEdit->text().simplified().remove(" ");
    //jsonObj.insert("barcode", QJsonValue(barcode));

    //if (m_verbose)
    //    qDebug() << "determine file output name ... ";

    //QString fileName;

    //// Use the output filename if it has a valid path
    //// If the path is invalid, use the directory where the application exe resides
    //// If the output filename is empty default output .json file is of the form
    //// <participant ID>_<now>_<devicename>.json
    ////
    //bool constructDefault = false;

    //// TODO: if the run mode is not debug, an output file name is mandatory, throw an error
    ////
    //if (m_outputFileName.isEmpty())
    //    constructDefault = true;
    //else
    //{
    //    QFileInfo info(m_outputFileName);
    //    QDir dir = info.absoluteDir();
    //    if (dir.exists())
    //        fileName = m_outputFileName;
    //    else
    //        constructDefault = true;
    //}
    //if (constructDefault)
    //{
    //    QDir dir = QCoreApplication::applicationDirPath();
    //    if (m_outputFileName.isEmpty())
    //    {
    //        QStringList list;
    //        list
    //            << barcode
    //            << QDate().currentDate().toString("yyyyMMdd")
    //            << m_manager.getGroup()
    //            << "test.json";
    //        fileName = dir.filePath(list.join("_"));
    //    }
    //    else
    //        fileName = dir.filePath(m_outputFileName);
    //}

    //QFile saveFile(fileName);
    //saveFile.open(QIODevice::WriteOnly);
    //saveFile.write(QJsonDocument(jsonObj).toJson());

    //if (m_verbose)
    //    qDebug() << "wrote to file " << fileName;

    //ui->statusBar->showMessage("Canadian Digit Triple Test data recorded.  Close when ready.");
}