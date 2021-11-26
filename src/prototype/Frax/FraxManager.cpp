#include "FraxManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>

FraxManager::FraxManager(QObject* parent):
	ManagerBase(parent)
{
}

void FraxManager::loadSettings(const QSettings& settings)
{
    // the full spec path name including exe name
    // eg., ../frax_module/blackbox.exe
    //
    QString exeName = settings.value("client/exe").toString();
    setExecutableName(exeName);
}

void FraxManager::saveSettings(QSettings* settings) const
{
    if (!m_executableName.isEmpty())
    {
        settings->setValue("client/exe", m_executablePath);
        if (m_verbose)
            qDebug() << "wrote exe fullspec path to settings file";
    }
}

QJsonObject FraxManager::toJsonObject() const
{
	return QJsonObject();
}

bool FraxManager::isDefined(const QString& exeName) const
{
    bool ok = false;
    if (!exeName.isEmpty())
    {
        QFileInfo info(exeName);
        if (info.exists() && info.isExecutable())
        {
            ok = true;
        }
    }
    return ok;
}

void FraxManager::setExecutableName(const QString&exeName)
{
    if (isDefined(exeName))
    {
        QFileInfo info(exeName);
        m_executableName = info.fileName();
        m_executablePath = info.filePath();
        m_inputPath = QDir(m_executablePath).filePath("../input.txt");
        m_oldInputPath = QDir(m_executablePath).filePath("../oldInput.txt");
        m_outputPath = QDir(m_executablePath).filePath("../output.txt");
    }
}

bool FraxManager::createInputsTxt()
{
    QString filePath = getInputFullPath();
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);

        stream << m_inputData["val1"].toString() + ","
            << m_inputData["val2"].toString() << ","
            << m_inputData["val3"].toString() << ","
            << m_inputData["val4"].toString() << ","
            << m_inputData["val5"].toString() << ","
            << m_inputData["val6"].toString() << ","
            << m_inputData["val7"].toString() << ","
            << m_inputData["val8"].toString() << ","
            << m_inputData["val9"].toString() << ","
            << m_inputData["val10"].toString() << ","
            << m_inputData["val11"].toString() << ","
            << m_inputData["val12"].toString() << ","
            << m_inputData["dxaHipTScore"].toString();

        file.close();
        qDebug() << "Wrote input.txt to " + filePath;
        return true;
    }
    return false;
}

void FraxManager::readOutputs()
{
    QFile file(getOutputFullPath());

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream instream(&file);
        QString line = instream.readLine();
        file.close();

        QStringList lineSplit = line.split(",");
        if (lineSplit.length() >= 17) {
            m_inputData["val1"] = lineSplit[0];
            m_inputData["val2"] = QString(lineSplit[1]).toDouble();
            m_inputData["val3"] = QString(lineSplit[2]).toDouble();
            m_inputData["val4"] = QString(lineSplit[3]).toDouble();
            m_inputData["val5"] = QString(lineSplit[4]).toDouble();
            m_inputData["val6"] = QString(lineSplit[5]).toDouble();
            m_inputData["val7"] = QString(lineSplit[6]).toDouble();
            m_inputData["val8"] = QString(lineSplit[7]).toDouble();
            m_inputData["val9"] = QString(lineSplit[8]).toDouble();
            m_inputData["val10"] = QString(lineSplit[9]).toDouble();
            m_inputData["val11"] = QString(lineSplit[10]).toDouble();
            m_inputData["val12"] = QString(lineSplit[11]).toDouble();
            m_inputData["dxaHipTScore"] = QString(lineSplit[12]).toDouble();
            m_outputData["fracRisk1"] = QString(lineSplit[13]).toDouble();
            m_outputData["fracRisk2"] = QString(lineSplit[14]).toDouble();
            m_outputData["fracRisk3"] = QString(lineSplit[15]).toDouble();
            m_outputData["fracRisk4"] = QString(lineSplit[16]).toDouble();
        }
    }
}

void FraxManager::clean()
{
    if (!m_outputPath.isEmpty())
    {
        QFile ofile(m_outputPath);
        ofile.remove();
    }
}

void  FraxManager::measure()
{
    // launch the process
    qDebug() << "starting process from measure";
    m_process.start();
    //m_process.waitForFinished();
}

void FraxManager::setInputs(const QMap<QString, QVariant>&)
{
}

void FraxManager::readOutput()
{
}

void FraxManager::clearData()
{
    m_test.reset();
    m_outputPath.clear();
    emit dataChanged();
}

void FraxManager::runBlackBoxExe()
{
    // Run blackbox.exe
    QProcess process;
    process.start(getExecutableFullPath());
    process.waitForFinished(2000);
    process.close();
}
