#pragma once

#include <QString>

#include "CRC8.h"

class BPMMessage
{
public:
	BPMMessage(quint8 msgID, quint8 data0);
	BPMMessage(quint8 msgID, quint8 data0, quint8 data1);
	BPMMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3);
	BPMMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3, quint8 crc);

	bool CheckCRCValid() { return CRC8::ValidCrc(msgBytes, 5, msgBytes[5]); };

	qint8 GetMsgId() { return msgBytes[0]; };
	qint8 GetData0() { return msgBytes[1]; };
	qint8 GetData1() { return msgBytes[2]; };
	qint8 GetData2() { return msgBytes[3]; };
	qint8 GetData3() { return msgBytes[4]; };
	qint8 GetCRC() { return msgBytes[5]; };
	QByteArray GetFullMsg() { return msgBytes; };
private:
	QByteArray msgBytes;
};

