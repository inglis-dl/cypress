#pragma once

#include <QThread>
#include <QQueue>
#include <QDebug>
#include <QTime>

#include "BPMMessage.h"

class CommunicationThread: public QThread
{
	Q_OBJECT
	void run() {
		while (*finishRunningPtr == false) {
			if (writeQueuePtr->count() > 0) {
				BPMMessage msg = writeQueuePtr->dequeue();
				qDebug() << msg.GetAsQString() << endl;
			}
			else {
				qDebug() << "Queue is empty" << endl;
			}
			sleep(5);
		}
		qDebug() << "Stopping thread work at " << QTime::currentTime() << endl;
	}
public:
	void SetValues(QQueue<BPMMessage>* writeQueue, QQueue<BPMMessage>* readQueue, bool* finishRunBool)
	{ 
		writeQueuePtr = writeQueue;
		readQueuePtr = readQueue;
		finishRunningPtr = finishRunBool;
	}
private:
	bool* finishRunningPtr;
	QQueue<BPMMessage>* writeQueuePtr;
	QQueue<BPMMessage>* readQueuePtr;
};

