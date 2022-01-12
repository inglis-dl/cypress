#include "TonometerManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QSettings>
#include <QStandardItemModel>

TonometerManager::TonometerManager(QObject* parent):
    ManagerBase(parent)
{
    setGroup("tonometer");

    m_inputKeyList << "date_of_birth";
    m_inputKeyList << "sex";
}

void TonometerManager::buildModel(QStandardItemModel *model) const
{
    QVector<QString> v_side({"left","right"});
    for(auto&& side : v_side)
    {
      int col = "left" == side ? 0 : 1;
      for(int row=0;row<8;row++)
      {
        TonometerMeasurement m; // = m_test.getMeasurement(side,row);
        QStandardItem* item = model->item(row,col);
        item->setData(m.toString(), Qt::DisplayRole);
      }
    }
}

void TonometerManager::loadSettings(const QSettings& settings)
{
    // the full spec path name including exe name
    // eg., ../frax_module/blackbox.exe
    //
    QString runnableName = settings.value(getGroup() + "/client/exe").toString();
    setRunnableName(runnableName);
}

void TonometerManager::saveSettings(QSettings* settings) const
{
    if (!m_runnableName.isEmpty())
    {
        settings->beginGroup(getGroup());
        settings->setValue("client/exe", m_runnableName);
        settings->endGroup();
        if (m_verbose)
            qDebug() << "wrote exe fullspec path to settings file";
    }
}

QJsonObject TonometerManager::toJsonObject() const
{
    QJsonObject json = m_test.toJsonObject();
    if("simulate" != m_mode)
    {
      //QFile ofile(m_outputFile);
      //ofile.open(QIODevice::ReadOnly);
      //QByteArray buffer = ofile.readAll();
      //json.insert("test_output_file",QString(buffer.toBase64()));
      json.insert("test_output_file_mime_type","txt");
    }
    return json;
}

bool TonometerManager::isDefined(const QString &runnableName) const
{
    if("simulate" == m_mode)
    {
       return true;
    }
    bool ok = false;
    if(!runnableName.isEmpty())
    {
        QFileInfo info(runnableName);
        if(info.exists() && info.isExecutable())
        {
            ok = true;
        }
    }
    return ok;
}

void TonometerManager::setRunnableName(const QString &runnableName)
{
    if(isDefined(runnableName))
    {
        QFileInfo info(runnableName);
        m_runnableName = runnableName;
        m_runnablePath = info.absolutePath();

        //TODO: change to db files
        //
        m_outputFile = QDir(m_runnablePath).filePath("output.txt");
        m_inputFile =  QDir(m_runnablePath).filePath("input.txt");
        m_temporaryFile = QDir(m_runnablePath).filePath("input_ORIG.txt");

        if(QFileInfo::exists(m_inputFile))
          configureProcess();
        else
          qDebug() << "ERROR: expected default input.txt does not exist";
    }
}

void TonometerManager::measure()
{
    if("simulate" == m_mode)
    {
        readOutput();
        return;
    }
    clearData();
    // launch the process
    qDebug() << "starting process from measure";
    m_process.start();
}

void TonometerManager::setInputData(const QMap<QString, QVariant> &input)
{
    if("simulate" == m_mode)
    {
        m_inputData["date_of_birth"] = "1965-12-17 00:00:00";
        m_inputData["sex"] = 0;
        return;
    }
    bool ok = true;
    for(auto&& x : m_inputKeyList)
    {
        if(!input.contains(x))
        {
            ok = false;
            qDebug() << "ERROR: missing expected input " << x;
            break;
        }
        else
            m_inputData[x] = input[x];
    }
    if(!ok)
        m_inputData.clear();
    else
        configureProcess();
}

void TonometerManager::readOutput()
{
    if("simulate" == m_mode)
    {
        qDebug() << "simulating read out";

        //TODO: impl left and right measurements
        //
        TonometerMeasurement m;
        m.setCharacteristic("name","IOPG");
        m.setCharacteristic("value", 1.0);
        m.setCharacteristic("units","-");
        m_test.addMeasurement(m);
        m.setCharacteristic("name","IOPG");
        m.setCharacteristic("value", 1.0);
        m.setCharacteristic("units","-");
        m_test.addMeasurement(m);


        for(auto&& x : m_inputData.toStdMap())
        {
          m_test.addMetaDataCharacteristic(x.first,x.second);
        }
        emit canWrite();
        emit dataChanged();
        return;
    }

    if (QProcess::NormalExit != m_process.exitStatus())
    {
        qDebug() << "ERROR: process failed to finish correctly: cannot read output";
        return;
    }
    else
        qDebug() << "process finished successfully";

    if(QFileInfo::exists(m_outputFile))
    {
        qDebug() << "found output txt file " << m_outputFile;
        //TODO: this could be read from a db file
        //
        m_test.fromFile(m_outputFile);
        if(m_test.isValid())
        {
            emit canWrite();
        }
        else
            qDebug() << "ERROR: input from file produced invalid test results";

        emit dataChanged();
    }
    else
        qDebug() << "ERROR: no output.txt file found";
}

void TonometerManager::configureProcess()
{
    /*
    if("simulate" == m_mode)
    {
        emit canMeasure();
        return;
    }
    */
    // The exe and input file are present
    //
    QFileInfo info(m_runnableName);
    QDir working(m_runnablePath);
    if (info.exists() && info.isExecutable() &&
        working.exists() && QFileInfo::exists(m_inputFile))
    {
        qDebug() << "OK: configuring command";

        m_process.setProcessChannelMode(QProcess::ForwardedChannels);
        m_process.setProgram(m_runnableName);
        m_process.setWorkingDirectory(m_runnablePath);

        qDebug() << "process working dir: " << m_runnablePath;

        // backup the original intput.txt
        if(QFileInfo::exists(m_temporaryFile))
        {
            QFile tfile(m_temporaryFile);
            tfile.remove();
        }
        QFile::copy(m_inputFile, m_temporaryFile);
        qDebug() << "wrote backup to " << m_temporaryFile;

        // generate the input.txt file content
        if(m_inputData.isEmpty())
        {
            qDebug() << "ERROR: no input data to write to input.txt";
            return;
        }
        QStringList list;
        for(auto&& x : m_inputKeyList)
        {
            if(m_inputData.contains(x))
              list << m_inputData[x].toString();
        }
        QString line = list.join(",");
        QFile ofile(m_inputFile);
        if(ofile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&ofile);
            stream << line;
            ofile.close();
            qDebug() << "populated the input.txt file " << m_inputFile;
            qDebug() << "content should be " << line;
        }
        else
        {
            qDebug() << "ERROR: failed writing to " << m_inputFile;
            return;
        }

        if("simulate" != m_mode)
        {
          connect(&m_process, &QProcess::started,
            this, [this]() {
                qDebug() << "process started: " << m_process.arguments().join(" ");
            });

          connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TonometerManager::readOutput);

          connect(&m_process, &QProcess::errorOccurred,
            this, [](QProcess::ProcessError error)
            {
                QStringList s = QVariant::fromValue(error).toString().split(QRegExp("(?=[A-Z])"), Qt::SkipEmptyParts);
                qDebug() << "ERROR: process error occured: " << s.join(" ").toLower();
            });

          connect(&m_process, &QProcess::stateChanged,
            this, [](QProcess::ProcessState state) {
                QStringList s = QVariant::fromValue(state).toString().split(QRegExp("(?=[A-Z])"), Qt::SkipEmptyParts);
                qDebug() << "process state: " << s.join(" ").toLower();

            });
        }
        emit canMeasure();
    }
    else
        qDebug() << "failed to configure process";
}

void TonometerManager::clearData()
{
    m_test.reset();
    emit dataChanged();
}

void TonometerManager::finish()
{
    //TODO: clear out db records
    //jdbc.update("DELETE FROM Measures WHERE PatientID = ?", patientDBId);
    //jdbc.update("DELETE FROM Patients WHERE PatientID = ?", patientDBId);
    //

    m_test.reset();
    if(QProcess::NotRunning != m_process.state())
    {
        m_process.close();
    }

    if(!m_outputFile.isEmpty() && QFileInfo::exists(m_outputFile))
    {
        qDebug() << "removing output file " << m_outputFile;
        QFile ofile(m_outputFile);
        ofile.remove();
        m_outputFile.clear();
    }

    if(!m_temporaryFile.isEmpty() && QFileInfo::exists(m_temporaryFile))
    {
        // remove the inputfile first
        QFile ifile(m_inputFile);
        ifile.remove();
        QFile::copy(m_temporaryFile, m_inputFile);
        qDebug() << "restored backup from " << m_temporaryFile;
        QFile tempFile(m_temporaryFile);
        tempFile.remove();
        m_temporaryFile.clear();
    }
}
