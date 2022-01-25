#ifndef BPMCOMMUNICATION_H
#define BPMCOMMUNICATION_H

#include <QObject>
#include <QHidDevice>

#include "BPMMessage.h"

class BPMCommunication: public QObject
{
	Q_OBJECT
public:
	explicit BPMCommunication(QHidDevice* bpm, QObject* parent = Q_NULLPTR);
public slots:
	void RunCommunicationLoop();

signals:
	void ReadingReady(const QByteArray &reading);

private:
	void WriteCommand(quint8 msgId, quint8 data0, quint8 data1 = 0x00, quint8 data2 = 0x00, quint8 data3 = 0x00);

	QHidDevice* m_bpm200;
};

#endif //BPMCOMMUNICATION_H