#include "Logger.h"

void Logger::Log(QString message)
{
	qDebug() << message;
}
