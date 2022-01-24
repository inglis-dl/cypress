#include <QObject>
#include <QHidDevice>
#include <QDebug>

#include "BPMMessage.h"

class BPM200: public QObject
{
public:
	explicit BPM200(QObject* parent = Q_NULLPTR);

	void SetVid(int vid) { m_vid = vid; };
	void SetPid(int pid) { m_pid = pid; };
	void Cycle();
	void Start();
	void Stop();
	void Clear();
	void Review();

	bool Connect();
	void Disconnect();
private:
	QHidDevice* m_bpm200;

	void WriteCommand(quint8 msgId, quint8 data0, quint8 data1 = 0x00, quint8 data2 = 0x00, quint8 data3 = 0x00);

	bool ConnectionInfoSet();
	int m_vid = 0;
	int m_pid = 0;
};

