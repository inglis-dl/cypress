#include "CognitiveTestManager.h"

#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>

CognitiveTestManager::CognitiveTestManager(QObject *parent) :
    ManagerBase(parent)
{
}

bool CognitiveTestManager::launch(const QString &userId, const QString &language)
{
	if (m_mode != "live") return true;

	try {
        QString command = QString("%1/%2").arg(m_ccbFolderPath, m_executable);
		QProcess process;
		QStringList arguments;
		arguments << "/u" + userId
			/*<< "/c" + dcsSiteName
			<< "/i" + interviewerId*/
			<< "/l" + language;
        process.setWorkingDirectory(m_ccbFolderPath);
		process.start(command, arguments);
		process.waitForFinished(100000000);
		process.close();
	}
	catch(...){
		// TODO: Determine better way of figuring out if the process failed or not
		return false;
	}
	
	return true;
}

bool CognitiveTestManager::moveResultsFile(const QString &outputFolderPath)
{
	if (m_mode != "live") return true;

    QString resultsDirPath = QString("%1/%2").arg(m_ccbFolderPath, m_resultsFolderName);
	QDir myDir(resultsDirPath);
	QStringList fileNames = myDir.entryList();
	bool fileFound = false;
    for (int i = 0; i < fileNames.count(); i++)
    {
		QString fileName = fileNames[i];
        if (fileName.endsWith(".csv"))
        {
			QString fullResultsFilePath = QString("%1/%2").arg(resultsDirPath, fileName);
			QFile file(fullResultsFilePath);

			// TODO: Determine if this should keep the filename or will a file name be provided from Cypress or above???
			QString movedResultsPath = QString("%1/%2").arg(outputFolderPath, fileName);
			file.rename(movedResultsPath);
			// TODO: Make logic smarter so that true is only set if the correct file is found 
			fileFound = true;
			//qDebug() << "Moved from " + fullResultsFilePath + " to " + movedResultsPath << endl;
		}
	}
	return fileFound;
}

void CognitiveTestManager::clearData()
{
    // TODO: use if this has context in a manager that launches and exe
    // consider implementing a different manager parent class
    //
}

void CognitiveTestManager::loadSettings(const QSettings &settings)
{
    m_ccbFolderPath = settings.value("client/ccbFolderPath").toString();
    m_executable = settings.value("client/executable").toString();
    m_resultsFolderName = settings.value("client/resultsFolderName").toString();
	
	// TODO: Should probably do something more regardles of verbose or not
	if (isVerbose()) {
        if (m_ccbFolderPath.isEmpty())
            qDebug() << "ERROR: No ccb folder path found in settings";
        if (m_executable.isEmpty())
            qDebug() << "ERROR: No executable name found in settings";
        if (m_resultsFolderName.isEmpty())
            qDebug() << "ERROR: No results folder name found in settings";
	}
}

void CognitiveTestManager::saveSettings(QSettings *settings) const
{
    if (!m_ccbFolderPath.isEmpty())
        settings->setValue("client/ccbFolderPath",m_ccbFolderPath);
    if (!m_executable.isEmpty())
        settings->setValue("client/executable",m_executable);
    if (!m_resultsFolderName.isEmpty())
        settings->setValue("client/resultsFolderName",m_resultsFolderName);
}

QJsonObject CognitiveTestManager::toJsonObject() const
{
    QJsonObject json;
//    json.insert("device",m_deviceData.toJsonObject());
    return json;
}
