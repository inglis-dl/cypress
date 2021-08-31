#pragma once

#include <QThread>
#include <QQueue>
#include <QDebug>
#include <QTime>
#include <QList>

#include "BPMMessage.h"
#include "BpmIO.h"

class CommunicationThread: public QThread
{
	Q_OBJECT
	void run() {
		while (*finishRunningPtr == false) {
			// Write all messages in the write queue to the BPM
			while(writeQueuePtr->count() > 0) {
				BPMMessage msg = writeQueuePtr->dequeue();
				bpmIO.Write(msg);
			}

			// Read messages from BPM and store in read queue
			QList<BPMMessage> bpmMessages = bpmIO.Read();
			if (bpmMessages.count() > 0) {
				for (BPMMessage msg : bpmMessages) {
					readQueuePtr->append(msg);
				}
				emit  BPMResponseRecieved();
			}
			
		}
		qDebug() << "Stopping thread work at " << QTime::currentTime() << endl;
	}
public:
	void SetValues(QQueue<BPMMessage>* writeQueue, QQueue<BPMMessage>* readQueue, bool* finishRunBool, bool isLive)
	{ 
		writeQueuePtr = writeQueue;
		readQueuePtr = readQueue;
		finishRunningPtr = finishRunBool;
		bpmIO.Setup(isLive);
	}
signals:
	void BPMResponseRecieved();
private:
	bool* finishRunningPtr;
	QQueue<BPMMessage>* writeQueuePtr;
	QQueue<BPMMessage>* readQueuePtr;
	BpmIO bpmIO;
};

