#include "BpmIO.h"

#include <QDebug>

void BpmIO::Setup(bool mIsLive)
{
	isLive = mIsLive;
	hid_init();
	bpm = hid_open(4279, 4660, 0);
	hid_set_nonblocking(bpm, 1);
}

void BpmIO::Finish()
{
	hid_exit();
}

QList<BPMMessage> BpmIO::Read()
{
    if (isLive) {
        return LiveRead();
    }
    else {
        return TestRead();
    }
    
}

void BpmIO::Write(BPMMessage message)
{
    if (isLive) {
        LiveWrite(message);
    }
    else {
        TestWrite(message);
    }
}

QList<BPMMessage> BpmIO::LiveRead()
{
	const int dataLength = 1024;
	unsigned char readData[dataLength];
	int numBytesReturned = 0;
	try {
		numBytesReturned = hid_read(bpm, readData, dataLength);
	}
	catch(...){
		Finish();
		Setup(isLive);
	}

	QList<BPMMessage> messages;
	if(numBytesReturned > 0) {
		qDebug() << ". NumBytes: " << numBytesReturned << endl;
		for (int i = 0; i + 7 < numBytesReturned;) {
			if (readData[i] = 2 && readData[i+7] == 3) {
				quint8 id = readData[i + 1];
				quint8 data0 = readData[i + 2];
				quint8 data1 = readData[i + 3];
				quint8 data2 = readData[i + 4];
				quint8 data3 = readData[i + 5];
				quint8 crc = readData[i + 6];
				BPMMessage msg(id, data0, data1, data2, data3, crc);
				messages.append(msg);
				qDebug() << "Message: " << msg.GetAsQString() << endl;
				qDebug() << "CRC Valid: " << msg.CheckCRCValid() << endl;
				i += 8;
			}
			else {
				qDebug() << i << " : " << readData[i] << " " << (i + 7) << " : " << readData[i+7] << endl;
				i++;
			}
		}
	}
	Wait::ForMilliSeconds(50);
    return messages;
}

QList<BPMMessage> BpmIO::TestRead()
{
    qDebug() << "(Test) Reading Messages" << endl;
    Wait::ForSeconds(5);
    return QList<BPMMessage>();
}

void BpmIO::LiveWrite(BPMMessage msg)
{
	const unsigned char data[] = {
		0x00, // Report #
		0x02, // STX
		msg.GetMsgId(), msg.GetData0(), msg.GetData1(), msg.GetData2(), msg.GetData3(), msg.GetCRC(), 
		0x03  // ETX
	};
	hid_write(bpm, data, 9); 
}

void BpmIO::TestWrite(BPMMessage message)
{
    qDebug() << "(Test) Writing message: " << message.GetAsQString() << endl;
}
