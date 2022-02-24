#ifndef BPMMESSAGE_H
#define BPMMESSAGE_H

#include <QString>

#include "CRC8.h"

class BPMMessage
{
public:
	BPMMessage(quint8 msgID, quint8 data0, quint8 data1 = 0x00, quint8 data2 = 0x00, quint8 data3 = 0x00);

	bool checkCRCValid(quint8 crcReceived) { return CRC8::ValidCrc(m_msgBytes, 5, crcReceived); };

	quint8 getMsgId() const { return m_msgBytes[0]; };
	quint8 getData0() const { return m_msgBytes[1]; };
	quint8 getData1() const { return m_msgBytes[2]; };
	quint8 getData2() const { return m_msgBytes[3]; };
	quint8 getData3() const { return m_msgBytes[4]; };
	QByteArray getFullMsg() const { return m_msgBytes; };
	QString getAsQString() const;
	static BPMMessage baseMessage() {
		return BPMMessage(0xff, 0xff);
	}

	QByteArray packMessage() const;

	static QByteArray createPackedMessage(quint8 msgID, quint8 data0, quint8 data1 = 0x00, quint8 data2 = 0x00, quint8 data3 = 0x00);
private:
	QByteArray m_msgBytes;
	const quint8 m_reportNum = 0x00;
	const quint8 m_stx = 0x02;
	const quint8 m_etx = 0x03;
};

#endif //BPMMESSAGE_H