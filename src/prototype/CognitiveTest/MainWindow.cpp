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

    // TODO: Add necessary UI elements
}

void MainWindow::run()
{
    QString userId;
    QString language;

    if(m_inputData.contains("barcode") && m_inputData["barcode"].isValid())
      userId = m_inputData["barcode"].toString();
    if(m_inputData.contains("language") && m_inputData["language"].isValid())
      language = m_inputData["language"].toString();

    // TODO: Should probably do something more regardles of verbose or not
    if (isVerbose())
    {
        if (userId.isEmpty())
            qDebug() << "ERROR: No barcode found in json input file";
        if (language.isEmpty())
            qDebug() << "ERROR: No language found in json input file";

        close();
    }

    m_manager.launch(userId, language);
    m_manager.moveResultsFile(m_outputFileName);
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
