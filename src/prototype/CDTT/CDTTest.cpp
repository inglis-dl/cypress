#include "CDTTest.h"
#include <QProcess>

void CDTTest::Run(InputsModel* inputs)
{
	QString command = "java";
	QProcess process;
	QStringList arguments;
	arguments << "-jar"
			  << "CDTTstereo.jar"
	          << inputs->participantId;
	process.setWorkingDirectory(inputs->path);
	process.start(command, arguments);
	process.waitForFinished(100000000);
	process.close();
}

void CDTTest::RetrieveData()
{
}
