#ifndef BPM200_H
#define BPM200_H

#include <QObject>
#include <QHidDevice>
#include <QDebug>
#include <QThread>
#include <QString>

#include "BPMMessage.h"
#include "BPMCommunication.h"

class BPM200: public QObject
{
	Q_OBJECT
	QThread CommThread;
public:
	explicit BPM200(QObject* parent = Q_NULLPTR);
	void SetupConnections();

	void SetConnectionInfo(int vid, int pid) { m_vid = vid; m_pid = pid; };

	void Connect();
	void Measure() { emit StartMeasurement(); };
	void Disconnect();
	
public slots:
	// slots for comm
	void ConnectionStatusReceived(bool connected) {
		//if (connected) emit StartMeasurement(); // TODO: REMOVE THIS
		emit ConnectionStatusReady(connected);
	}
	void MeasurementReceived(const int& sbp, const int& dbp, const int& pulse, const const QDateTime& start,
		const QDateTime& end, const int& readingNum, const bool& isAverage, const bool& done) { MeasurementReady(sbp, dbp, pulse, start, end, readingNum, isAverage, done); }
	void AbortComplete(bool successful);
signals:
	// Signals to comm
	void AttemptConnection(const int vid, const int pid);
	void StartMeasurement();
	void AbortMeasurement(QThread* uiThread);

	// signals to manager
	void ConnectionStatusReady(bool connected);
	void MeasurementReady(const int& sbp, const int& dbp, const int& pulse, const const QDateTime& start,
		const QDateTime& end, const int& readingNum, const bool& isAverage, const bool& done);
private:
	BPMCommunication* comm;
	bool ConnectionInfoSet();
	int m_vid = 0;
	int m_pid = 0;
	bool m_aborted = false;
	bool m_connectionsSet = false;
};

#endif //BPM200_H