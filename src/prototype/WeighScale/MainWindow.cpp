#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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
  m_manager.setVerbose(m_verbose);

  readInput();

  // Read the .ini file for cached local and peripheral device addresses
  //
  QDir dir = QCoreApplication::applicationDirPath();
  QSettings settings(dir.filePath("weighscale.ini"), QSettings::IniFormat);
  m_manager.loadSettings(settings);

  // Connect button to connect a controller to a verified peripheral device
  //
  ui->connectButton->setEnabled(false);

  // Save button to store measurement and device info to .json
  //
  ui->saveButton->setEnabled(false);

  connect(&m_manager, &WeighScaleManager::weightChanged,
          ui->weightLineEdit, &QLineEdit::setText);

  connect(&m_manager, &WeighScaleManager::datetimeChanged,
          ui->dateTimeLineEdit, &QLineEdit::setText);

}

void MainWindow::setInputKeys(const QList<QString> &keys)
{
   m_inputData.clear();
   for(int i=0;i<keys.size();i++)
       m_inputData.insert(keys[i],QVariant());
}

void MainWindow::setOutputKeys(const QList<QString> &keys)
{
   m_outputData.clear();
   for(int i=0;i<keys.size();i++)
       m_outputData.insert(keys[i],QVariant());
}

void MainWindow::run()
{
//    m_manager.scanDevices();
}

void MainWindow::finish()
{
//    m_manager.scanDevices();
}

void MainWindow::readInput()
{
    // TODO: if the run mode is not debug, an input file name is mandatory, throw an error
    //
    if(m_inputFileName.isEmpty())
    {
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

      QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
      QJsonObject o = d.object();
      QMapIterator<QString,QVariant> it(m_inputData);
      QList<QString> keys = m_inputData.keys();
      for(int i=0;i<keys.size();i++)
      {
          QJsonValue v = o.value(keys[i]);
          // TODO: error report all missing expected key values
          //
          if(!v.isUndefined())
              m_inputData[keys[i]] = v.toVariant();
      }
    }
}

void MainWindow::writeOutput()
{
   if(m_verbose)
       qDebug() << "begin write process ... ";

   QMap<QString,QVariant> data = m_manager.getData();

   if(m_verbose)
       qDebug() << "data retrieved from device ... ";

   // copy in expected device data
   //
   QMap<QString,QVariant>::const_iterator it = data.constBegin();
   while(it != data.constEnd())
   {
     if(m_outputData.contains(it.key()))
        m_outputData[it.key()] = it.value();
     ++it;
   }

   // get the most recent input barcode
   //
   m_outputData["Barcode"] = ui->barcodeLineEdit->text().simplified().remove(" ");

   // Create a json object with measurement key value pairs
   //
   QJsonObject json;
   it = m_outputData.constBegin();
   int missingCount = 0;
   while(it != m_outputData.constEnd())
   {
     if(m_outputData.contains(it.key()))
         json.insert(it.key(),QJsonValue::fromVariant(it.value()));
     else
         missingCount++;
     ++it;
   }

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
         list << m_outputData["Barcode"].toString();
         list << QDate().currentDate().toString("yyyyMMdd");
         list << "lowenergythermometer.json";
         fileName = dir.filePath( list.join("_") );
       }
       else
         fileName = dir.filePath( m_outputFileName );
   }

   QFile saveFile( fileName );
   saveFile.open(QIODevice::WriteOnly);
   saveFile.write(QJsonDocument(json).toJson());

   if(m_verbose)
       qDebug() << "wrote to file " << fileName;

   ui->statusBar->showMessage("Weigh scale data recorded.  Close when ready.");
}
