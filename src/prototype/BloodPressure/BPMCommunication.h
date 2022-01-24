#include <QObject>

class BPMCommunication: public QObject
{
	Q_OBJECT
public slots:
	void RunCommunicationLoop();

signals:
	void ReadingReady(const QByteArray &reading);
};

