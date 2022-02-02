#ifndef BPMCOMMUNICATION_H
#define BPMCOMMUNICATION_H

#include <QObject>
#include <QHidDevice>
#include <QString>
#include <QQueue>
#include <functional>
#include <iostream>
using namespace std;

#include "BPMMessage.h"

class BPMCommunication: public QObject
{
	Q_OBJECT
public:
	explicit BPMCommunication(QObject* parent = Q_NULLPTR);
public slots:
	void Connect(const int vid, const int pid);
	void Measure();
	void Abort();

signals:
	void ConnectionStatus(const bool connected);
	void MeasurementReady(const QString measurement);

private:
	bool Start();
	bool Stop();
	bool Cycle();
	bool Clear();
	bool Review();
	void WriteCommand(const quint8 msgId, const quint8 data0, const quint8 data1 = 0x00, const quint8 data2 = 0x00, const quint8 data3 = 0x00);
	void Read();

	bool TimedReadLoop(const int seconds, const function<bool()> func);
	bool AckCheck(const int expectedData0, const QString logName);

	QHidDevice* m_bpm200;
	QQueue<BPMMessage>* m_msgQueue;


	// The last reported cycle time
	int cycleTime = -1;

	// The last reported cuff pressure
	int cuffPressure = -1;

	// The last reported reading number
	int readingNumber = -1;

	// The last result code
	int resultCode = -1;
};

#endif //BPMCOMMUNICATION_H
