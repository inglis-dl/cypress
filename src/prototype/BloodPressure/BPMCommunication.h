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
	void connectToBpm(const int vid, const int pid);
	void measure();
	void abort(QThread* uiThread);

signals:
	void connectionStatus(const bool connected);
	void versionInfoAvailable(const QString version);
	void measurementReady(const int& sbp, const int& dbp, const int& pulse, const QDateTime& start, const QDateTime& end, const int& readingNum);
	void averageReady(const int& sbp, const int& dbp, const int& pulse);
	void finalReviewReady(const int& sbp, const int& dbp, const int& pulse);
	void measurementFailed(); // TODO: hookup
	void abortFinished(bool successful);

private:
	bool attemptConnectionToBpm();
	bool startBpm();
	bool stopBpm();
	bool cycleBpm();
	bool clearBpm();
	bool reviewBpm();
	bool handshakeBpm();
	bool timedWriteBpm(const quint8 msgId, const quint8 data0, const quint8 data1 = 0x00, const quint8 data2 = 0x00, const quint8 data3 = 0x00);
	void readFromBpm();

	bool timedLoop(const int timeout, const function<bool()> func, const QString debugName = "");
	bool timedReadLoop(const int timeout, const function<bool()> func, const QString debugName = "");
	bool ackCheck(const int expectedData0, const QString logName);
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
