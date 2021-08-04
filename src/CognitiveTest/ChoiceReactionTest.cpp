#include "ChoiceReactionTest.h"

void ChoiceReactionTest::Run(InputsModel* inputs)
{
	QString command = inputs->path + "/CCB";
	QProcess process;
	QStringList arguments;
	arguments << "/u" + inputs-> userID 
			  << "/c" + inputs-> dcsSiteName 
			  << "/i" + inputs-> interviewerID
			  << "/l" + inputs-> language;
	process.setWorkingDirectory(inputs->path);
	process.start(command, arguments);
	process.waitForFinished(100000000);
	process.close();
}

void ChoiceReactionTest::MoveResultsFile(QString ccbDirPath)
{
	QString resultsDirPath = ccbDirPath + "/results";
	QDir myDir(resultsDirPath);
	QStringList fileNames = myDir.entryList();
	for (int i = 0; i < fileNames.count(); i++) {
		QString fileName = fileNames[i];
		if (fileName.endsWith(".csv")) {
			QString fullResultsFilePath = resultsDirPath + "/" + fileName;
			QFile file(fullResultsFilePath);

			QString movedResultsPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + fileName;
			file.rename(movedResultsPath);
			//qDebug() << "Moved from " + fullResultsFilePath + " to " + movedResultsPath << endl;
		}
	}
}
