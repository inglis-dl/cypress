#include "EasyOnPcHelper.h"


void EasyOnPcHelper::LaunchEasyOnPc()
{
	QString dir = "C:/Program Files (x86)/ndd Medizintechnik/Easy on-PC/Application";
	QString cmd = QString("\"C:\\Program Files (x86)\\ndd Medizintechnik\\Easy on-PC\\Application\\EasyWarePro.exe\"");
	QProcess process;
	qDebug() << "Launching easy on pc" << endl;
	process.setWorkingDirectory(dir);
	process.start(cmd);
	bool launched = process.waitForFinished(100000000);
	process.close();
	qDebug() << "Easy on pc launched = " << launched << endl;
}

void EasyOnPcHelper::ResetFiles()
{
	QString onyxOutFilePath = "C:/ProgramData/ndd/Easy on-PC/OnyxOut.xml";

	if (QFile::exists(onyxOutFilePath)) {
		QFile::remove(onyxOutFilePath);
	}
}
