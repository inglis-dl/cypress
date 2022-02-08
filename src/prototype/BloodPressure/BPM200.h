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

	void SetConnectionInfo(int vid, int pid) { m_vid = vid; m_pid = pid; };

	void Connect();
	void Disconnect();
	BPMCommunication* comm;
	
public slots:
	void ReceiveConnectionStatus(bool connected);
	void ReceiveMeasurement(QString measurement);
	void AbortComplete(bool successful);
signals:
	void AttemptConnection(const int vid, const int pid);
	void StartMeasurement();
	void AbortMeasurement(QThread* uiThread);

	void AskForThreadId();
private:
	bool ConnectionInfoSet();
	int m_vid = 0;
	int m_pid = 0;
	bool m_aborted = false;
};

#endif //BPM200_H