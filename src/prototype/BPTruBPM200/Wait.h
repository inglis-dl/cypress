#pragma once

#include <QTime>
#include <QEventLoop>
#include <QCoreApplication>

class Wait {
public:
	static void ForSeconds(int seconds) {
		QTime dieTime = QTime::currentTime().addSecs(seconds);
		WaitUntil(dieTime);
	}
	static void ForMilliSeconds(int milliSeconds) {
		QTime dieTime = QTime::currentTime().addMSecs(milliSeconds);
		WaitUntil(dieTime);
	}
private:
	static void WaitUntil(QTime dieTime) {
		while (QTime::currentTime() < dieTime)
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}
};
