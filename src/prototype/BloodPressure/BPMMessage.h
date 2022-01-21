#include <QString>
#include <QList>

#include "CRC8.h"

class BPMMessage
{
public:
	BPMMessage(quint8 msgID, quint8 data0, quint8 data1 = 0x00, quint8 data2 = 0x00, quint8 data3 = 0x00);

	bool CheckCRCValid(quint8 crcReceived) { return CRC8::ValidCrc(msgBytes, 5, crcReceived); };

	quint8 GetMsgId() { return msgBytes[0]; };
	quint8 GetData0() { return msgBytes[1]; };
	quint8 GetData1() { return msgBytes[2]; };
	quint8 GetData2() { return msgBytes[3]; };
	quint8 GetData3() { return msgBytes[4]; };
	QByteArray GetFullMsg() { return msgBytes; };
	QString GetAsQString();
	static BPMMessage BaseMessage() {
		return BPMMessage(0xff, 0xff);
	}

	QByteArray PackMessage();
private:
	QByteArray msgBytes;
};

