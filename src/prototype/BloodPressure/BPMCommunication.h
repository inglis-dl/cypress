#ifndef BPMCOMMUNICATION_H
#define BPMCOMMUNICATION_H

#include <QObject>
#include <QDateTime>
#include <QHidDevice>
#include <QString>
#include <QQueue>
#include <functional>
#include <iostream>
#include <QThread>
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
	void Abort(QThread* uiThread);

signals:
	void ConnectionStatus(const bool connected);
	void VersionInfoAvailable(const QString version);
	void MeasurementReady(const int& sbp, const int& dbp, const int& pulse, const QDateTime& start, const QDateTime& end, const int& readingNum, 
		const bool& isAverage = false, const bool& done = false);
	void AverageReady(const int& sbp, const int& dbp, const int& pulse);
	void FinalReviewReady(const int& sbp, const int& dbp, const int& pulse);
	void MeasurementFailed();
	void ConnectionLost();
	void AbortFinished(bool successful);

private:
	bool ConnectToBpm();
	bool Start();
	bool Stop();
	bool Cycle();
	bool Clear();
	bool Review();
	bool Handshake();
	bool TimedWrite(const quint8 msgId, const quint8 data0, const quint8 data1 = 0x00, const quint8 data2 = 0x00, const quint8 data3 = 0x00);
	void Read();

	bool TimedLoop(const int timeout, const function<bool()> func, const QString debugName = "");
	bool TimedReadLoop(const int timeout, const function<bool()> func, const QString debugName = "");
	bool AckCheck(const int expectedData0, const QString logName);
	void resetValues();

	QHidDevice* m_bpm200;
	QQueue<BPMMessage>* m_msgQueue;

	// The vid and pid for connecting to the bpm
	int m_vid = -1;
	int m_pid = -1;

	void startReading();
	void endReading(const int& sbp, const int& dbp, const int& pulse);
	QDateTime m_readingStartTime = QDateTime::fromMSecsSinceEpoch(0);


	// The last reported cycle time
	int m_cycleTime = -1;

	// The last reported cuff pressure
	int m_cuffPressure = -1;

	// The last reported reading number
	//int m_readingNumber = -1;
	int m_readingNumberCalc = 0;

	// The last result code
	//int m_resultCode = -1;

	// True if the bpm is currently taking measurements
	bool m_measuring = false;

	bool m_aborted = false;
};

#endif //BPMCOMMUNICATION_H
