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
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_verbose(false)
{
    ui->setupUi(this);

    // allocate 1 columns x 4 rows of frax measurement items
    //

    for(int i = 0; i < 4; i++)
    {
      QStandardItem* item = new QStandardItem();
      m_model.setItem(i, 0, item);
    }

    m_model.setHeaderData(0, Qt::Horizontal, "Frax 10 Year Fracture Risk Probabilities", Qt::DisplayRole);
    ui->testdataTableView->setModel(&m_model);

    ui->testdataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->testdataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->verticalHeader()->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
    m_manager.setVerbose(m_verbose);
    m_manager.setMode(m_mode);

    // Read inputs required to launch frax test
    //
    readInput();

    // Populate barcode display
    //
    if(m_inputData.contains("barcode") && m_inputData["barcode"].isValid())
       ui->barcodeLineEdit->setText(m_inputData["barcode"].toString());
    else
       ui->barcodeLineEdit->setText("00000000"); // dummy

    QDir dir = QCoreApplication::applicationDirPath();
    qDebug() << "Dir: " << dir;
    QSettings settings(dir.filePath("frax.ini"), QSettings::IniFormat);

    // have the manager build the inputs from the input json file
    m_manager.setInputData(m_inputData);

    // Select the location of blackbox.exe
    //
    ui->openButton->setEnabled(false);

    // Launch blackbox.exe
    //
    ui->measureButton->setEnabled(false);

    // Save button to store measurement and device info to .json
    //
    ui->saveButton->setEnabled(false);

    // Close the application
    //
    ui->closeButton->setEnabled(true);

    // blackbox.exe was found or set up successfully
    //
    connect(&m_manager, &FraxManager::canMeasure,
        this, [this]() {
            ui->statusBar->showMessage("Ready to measure...");
            ui->measureButton->setEnabled(true);
            ui->saveButton->setEnabled(false);
        });

    // Request a measurement from the device (run blackbox.exe)
    //
    connect(ui->measureButton, &QPushButton::clicked,
        &m_manager, &FraxManager::measure);

    // Update the UI with any data
    //
    connect(&m_manager, &FraxManager::dataChanged,
        this, [this]() {
      QHeaderView *h = ui->testdataTableView->horizontalHeader();
      h->setSectionResizeMode(QHeaderView::Fixed);
      m_manager.buildModel(&m_model);

      QSize ts_pre = ui->testdataTableView->size();
      h->resizeSections(QHeaderView::ResizeToContents);
      ui->testdataTableView->setColumnWidth(0, h->sectionSize(0));
      ui->testdataTableView->resize(
        h->sectionSize(0) + 1,
        4*(ui->testdataTableView->rowHeight(0)) + 1 +
        h->height());
      QSize ts_post = ui->testdataTableView->size();
      int dx = ts_post.width() - ts_pre.width();
      int dy = ts_post.height() - ts_pre.height();
      this->resize(this->width() + dx, this->height() + dy);
    });

    // All measurements received: enable write test results
    //
    connect(&m_manager, &FraxManager::canWrite,
        this, [this]() {
            ui->statusBar->showMessage("Ready to save results...");
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

    // read the path to blackbox.exe
    //
    m_manager.loadSettings(settings);

    // validate the presence of blackbox.exe and enable
    // file selection as required
    //
    QString exeName = m_manager.getExecutableName();
    if (!m_manager.isDefined(exeName))
    {
        ui->openButton->setEnabled(true);
        QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the exe by clicking Open and browsing to the "
                "required executable (blackbox.exe) and selecting the file.  If the executable "
                "is valid click the Run button to start the test otherwise check the installation."));
    }

    connect(ui->openButton, &QPushButton::clicked,
        this, [this]() {
            QString fileName =
                QFileDialog::getOpenFileName(
                    this, tr("Open File"),
                    QCoreApplication::applicationDirPath(),
                    tr("Applications (*.exe, *)"));
            if (m_manager.isDefined(fileName))
            {
                m_manager.setExecutableName(fileName);
                ui->measureButton->setEnabled(true);
                ui->saveButton->setEnabled(false);
            }
            else
                qDebug() << fileName << " not a valid executable";
        });
}

void MainWindow::run()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_verbose)
        qDebug() << "close event called";

    // remove all csv files from the results folder
    m_manager.clean();

    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath("frax.ini"), QSettings::IniFormat);
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
            list << barcode;
            list << QDate().currentDate().toString("yyyyMMdd");
            list << "fraxtest.json";
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

    ui->statusBar->showMessage("Frax data recorded.  Close when ready.");
}
