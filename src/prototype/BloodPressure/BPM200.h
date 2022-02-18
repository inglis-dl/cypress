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
	void setupConnections();

	void SetConnectionInfo(int vid, int pid) { m_vid = vid; m_pid = pid; };

	void connectToBpm();
	void measure() { emit startMeasurement(); };
	void disconnect();
	
public slots:
	// slots for comm
	void connectionStatusReceived(bool connected) {
		//if (connected) emit StartMeasurement(); // TODO: REMOVE THIS
		emit connectionStatusReady(connected);
	}
	void measurementReceived(const int& sbp, const int& dbp, const int& pulse, const const QDateTime& start, 
		const QDateTime& end, const int& readingNum) { measurementReady(sbp, dbp, pulse, start, end, readingNum); }
	void averageRecieved(const int& sbp, const int& dbp, const int& pulse) { averageReady(sbp, dbp, pulse); }
	void finalReviewRecieved(const int& sbp, const int& dbp, const int& pulse) { finalReviewReady(sbp, dbp, pulse); }
	void abortComplete(bool successful);
signals:
	// Signals to comm
	void attemptConnection(const int vid, const int pid);
	void startMeasurement();
	void abortMeasurement(QThread* uiThread);

	// signals to manager
	void connectionStatusReady(bool connected);
	void measurementReady(const int& sbp, const int& dbp, const int& pulse, const const QDateTime& start, const QDateTime& end, const int& readingNum);
	void averageReady(const int& sbp, const int& dbp, const int& pulse);
	void finalReviewReady(const int& sbp, const int& dbp, const int& pulse);
private:
	BPMCommunication* comm;
	bool connectionInfoSet();
	int m_vid = 0;
	int m_pid = 0;
	bool m_aborted = false;
	bool m_connectionsSet = false;
};

#endif //BPM200_H