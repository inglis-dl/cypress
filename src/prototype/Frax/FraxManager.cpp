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
        settings->setValue("client/exe", m_executableName);
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

void FraxManager::clearData()
{
    //m_test.reset();
    emit dataChanged();
}
