#ifndef BPM200_H
#define BPM200_H

#include <QObject>
#include <QThread>
#include <QUsb>
#include <QList>

QT_FORWARD_DECLARE_CLASS(BPMCommunication)

class BPM200: public QObject
{
	Q_OBJECT
	QThread m_commThread;
private:
	const int m_vid = 4279;
public:
	explicit BPM200(QObject* parent = Q_NULLPTR);
	void setupConnections();

	void setConnectionInfo(int pid) { m_pid = pid; };

	void connectToBpm();
	void measure() { emit startMeasurement(); };
	void disconnect();

	QList<int> findAllPids();
	bool connectionInfoSet() const { return m_pid > 0; }
	int getPid() const { return m_pid; }
	int getVid() const { return m_vid; }
	
public slots:
	// slots for comm
	void connectionStatusReceived(bool connected) {
		//if (connected) emit StartMeasurement(); // TODO: REMOVE THIS
		emit connectionStatusReady(connected);
	}
	void measurementReceived(const int& sbp, const int& dbp, const int& pulse, const const QDateTime& start, 
		const QDateTime& end, const int& readingNum) { emit measurementReady(sbp, dbp, pulse, start, end, readingNum); }
	void averageRecieved(const int& sbp, const int& dbp, const int& pulse) { emit averageReady(sbp, dbp, pulse); }
	void finalReviewRecieved(const int& sbp, const int& dbp, const int& pulse) { emit finalReviewReady(sbp, dbp, pulse); }
	void abortComplete(bool successful);
	void errorRecieved(const QString& error) { emit sendError(error); }
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
	void sendError(const QString& error);
private:
	BPMCommunication* m_comm;
	int m_pid = 0;
	bool m_aborted = false;
	bool m_connectionsSet = false;
	QUsb m_usb;
};

#endif //BPM200_H