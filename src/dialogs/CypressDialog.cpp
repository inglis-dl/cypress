#include "CypressDialog.h"

#include <QCoreApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>

CypressDialog::CypressDialog(QWidget *parent)
    : QDialog(parent)
    , m_verbose(false)
{
}

void CypressDialog::initialize()
{
  initializeModel();
  initializeConnections();
}

// run should only be called AFTER the user inputs a valid barcode
//
void CypressDialog::run()
{
    m_manager->setVerbose(m_verbose);
    m_manager->setMode(m_mode);

    // Read the .ini file for cached device data
    //
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager->getGroup() + ".ini"), QSettings::IniFormat);
    m_manager->loadSettings(settings);

    // Pass the input to the manager for verification
    //
    m_manager->setInputData(m_inputData);

    m_manager->start();
}

void CypressDialog::closeEvent(QCloseEvent *event)
{
    if(m_verbose)
        qDebug() << "close event called";
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager->getGroup() + ".ini"), QSettings::IniFormat);
    m_manager->saveSettings(&settings);
    m_manager->finish();
    event->accept();
}

void CypressDialog::readInput()
{
    // TODO: if the run mode is not debug, an input file name is mandatory, throw an error
    //
    if(m_inputFileName.isEmpty())
    {
        if("simulate" == m_mode)
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
    }
    else
        qDebug() << m_inputFileName << " file does not exist";
}

void CypressDialog::writeOutput()
{
   if(m_verbose)
       qDebug() << "begin write process ... ";

   QJsonObject jsonObj = m_manager->toJsonObject();

   QString barcode = getVerificationBarcode();
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
           << m_manager->getInputDataValue("barcode").toString()
           << QDate().currentDate().toString("yyyyMMdd")
           << m_manager->getGroup()
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
}
