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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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
    m_manager.setVerbose(m_verbose);
    m_manager.setMode(m_mode);

    // Read inputs required to launch cognitive test
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
    QSettings settings(dir.filePath("cognitive.ini"), QSettings::IniFormat);

    // read the path to C:\Program Files (x86)\Cardiff_University\CCB\CCB.exe
    //
    m_manager.loadSettings(settings);

    // Select the location of CCB.exe
    //
    ui->openButton->setEnabled(false);

    // Launch CCB.exe
    //
    ui->measureButton->setEnabled(false);

    // Save button to store measurement and device info to .json
    //
    ui->saveButton->setEnabled(false);

    // Close the application
    //
    ui->closeButton->setEnabled(true);

    // CCB.exe was found or set up successfully
    //
    connect(&m_manager, &CognitiveTestManager::canMeasure,
            this,[this](){
        ui->statusBar->showMessage("Ready to measure...");
        ui->measureButton->setEnabled(true);
        ui->saveButton->setEnabled(false);
    });

    // Request a measurement from the device (run CCB.exe)
    //
    connect(ui->measureButton, &QPushButton::clicked,
          &m_manager, &CognitiveTestManager::measure);

    // All measurements received: enable write test results
    //
    connect(&m_manager, &CognitiveTestManager::canWrite,
            this,[this](){
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

    // validate the presence of CCB.exe and enable
    // file selection as required
    //
    QString exeName = m_manager.getExecutableName();
    if(!m_manager.isDefined(exeName))
    {
        ui->openButton->setEnabled(true);
        QMessageBox::warning(
          this, QApplication::applicationName(),
          tr("Select the exe by clicking Open and browsing to the "
          "required executable (CCB.exe) and selecting the file.  If the executable "
          "is valid click the Run button to start the test otherwise check the installation."));
    }

    connect(ui->openButton, &QPushButton::clicked,
            this,[this](){
        QString fileName =
          QFileDialog::getOpenFileName(
            this, tr("Open File"),
                    QCoreApplication::applicationDirPath(),
                    tr("Applications (*.exe)"));
        if(m_manager.isDefined(fileName))
        {
            m_manager.setExecutableName(fileName);
            ui->measureButton->setEnabled(true);
            ui->saveButton->setEnabled(false);
        }
    });
}

void MainWindow::run()
{
    // TODO: create the logic and code for bypassing the UI here
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(m_verbose)
        qDebug() << "close event called";
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath("cognitivetest.ini"), QSettings::IniFormat);
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
