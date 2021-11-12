#include "CognitiveTestManager.h"

bool CognitiveTestManager::LaunchTest(QString CCBFolderPath, QString userId, QString dcsSiteName, QString interviewerId, QString language)
{
	if (m_mode != "live") return true;

	try {
		QString executable = "CCB";
		QString command = QString("%1/%2").arg(CCBFolderPath, executable);
		QProcess process;
		QStringList arguments;
		arguments << "/u" + userId
			<< "/c" + dcsSiteName
			<< "/i" + interviewerId
			<< "/l" + language;
		process.setWorkingDirectory(CCBFolderPath);
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

bool CognitiveTestManager::MoveResultsFile(QString ccbFolderPath, QString outputFolderPath)
{
	if (m_mode != "live") return true;

	QString resultsDirPath = QString("%1/results").arg(ccbFolderPath);
	QDir myDir(resultsDirPath);
	QStringList fileNames = myDir.entryList();
	bool fileFound = false;
	for (int i = 0; i < fileNames.count(); i++) {
		QString fileName = fileNames[i];
		if (fileName.endsWith(".csv")) {
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
