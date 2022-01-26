#include "BPMCommunication.h"

#include <QTime>
#include <QCoreApplication>
#include <QEventLoop>

BPMCommunication::BPMCommunication(QObject* parent) 
	: m_bpm200(new QHidDevice())
	, m_msgQueue(new QQueue<BPMMessage>())
{
}

void BPMCommunication::Connect(int vid, int pid) {
	// Read and write from bpm here
	if (m_bpm200->isOpen()) {
		qDebug() << "Already connected";
		emit ConnectionStatus(true);
		return;
	}

	bool successful = m_bpm200->open(vid, pid);
	qDebug() << "Connection attempt with bpm: " << (successful ? "Successful": "Failed");
	emit ConnectionStatus(successful);
}

void BPMCommunication::Measure() {
	// Read and write from bpm here
	int cycleTime = -1;
	while (cycleTime != 5) {
		cycleTime = Cycle();
	}
}

void BPMCommunication::Abort() {
	// Read and write from bpm here
}

void BPMCommunication::Start()
{
    qDebug() << "BPMComm: Start";
	WriteCommand(0x11, 0x04);
	//while (true) {
	//	

	//	// Read messages from BPM and store in read queue
	//	const int dataLength = 64;
	//	QByteArray readData[dataLength];
	//	m_bpm200->read(readData, dataLength, 1000);
	//	qDebug() << readData->toStdString();
	//}
}

void BPMCommunication::Stop()
{
    qDebug() << "BPMComm: Stop";
    WriteCommand(0x11, 0x01);
}

int BPMCommunication::Cycle()
{
	qDebug() << "BPMComm: Cycle";
	WriteCommand(0x11, 0x03);

	QTime currTime = QTime::currentTime();
	QTime dieTime = currTime.addSecs(30);
	while (currTime < dieTime) {
		Read();
		while (m_msgQueue->isEmpty() == false) {
			BPMMessage nextMessage = m_msgQueue->dequeue();
			if (nextMessage.GetMsgId() == 6 && nextMessage.GetData0() == 3) {
				int cycleTime = nextMessage.GetData1();
				qDebug() << "Ack received for cycle. Cycle Time set to " << cycleTime << endl;
				return cycleTime;
			}
			else {
				qDebug() << "WARNING: Received message that is not a cycle ack " << nextMessage.GetAsQString() << endl;
			}
		}

		currTime = QTime::currentTime();
		qDebug() << currTime << endl;
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}

	// return -1 meaning no ack received
	qDebug() << "Did not receive an ack after calling cycle" << endl;
	return -1;
}

void BPMCommunication::Clear()
{
}

void BPMCommunication::Review()
{
}

void BPMCommunication::WriteCommand(quint8 msgId, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    QByteArray buf = BPMMessage::CreatePackedMessage(msgId, data0, data1, data2, data3);
	bool connected = m_bpm200->isOpen();
    m_bpm200->write(&buf, buf.size());
}

void BPMCommunication::Read()
{
	// Read from the BPM
	const int dataLength = 64;
	QByteArray readData[dataLength];
	int numBytesReturned = m_bpm200->read(readData, dataLength, 300);

	// Stop here if no data was read in
	if (numBytesReturned <= 0) return;

	// Convert read in QByteArray to a QByteArray with values that can be used as quint8
	// NOTE: Typically all the data is stored in index 0 of readData which is why the 
	//		 toHex then FromHex needs to be done. However this code does not assume that 
	//		 will always be the case. It still checks each byte in readData up to index 
	//       numBytesReturned - 1 just incase their is data in any of thoses indecies 
	QByteArray bytes = QByteArray::fromHex(readData[0].toHex());
	for (int i = 1; i < numBytesReturned; i++) {
		QByteArray tempBytes = QByteArray::fromHex(readData[i].toHex());
		bytes.append(tempBytes);
	}

	// Convert bytes to BPMMessages
	int numBytes = bytes.size();
	if (numBytes > 0) {
		qDebug() << ". NumBytes: " << numBytes << endl;
		for (int i = 0; i + 7 < numBytes;) {
			quint8 firstByte = bytes[i];
			quint8 lastByte = bytes[i + 7];
			if (firstByte == 2 && lastByte == 3) {
				quint8 id = bytes[i + 1];
				quint8 data0 = bytes[i + 2];
				quint8 data1 = bytes[i + 3];
				quint8 data2 = bytes[i + 4];
				quint8 data3 = bytes[i + 5];
				quint8 crc = bytes[i + 6];
				BPMMessage msg(id, data0, data1, data2, data3);
				bool crcValid = msg.CheckCRCValid(crc);
				qDebug() << "Message: " << msg.GetAsQString() << endl;
				qDebug() << "CRC Valid: " << crcValid << endl;
				if (crcValid) {
					m_msgQueue->enqueue(msg);
				}
				i += 8;
			}
			else {
				qDebug() << i << " : " << readData[i] << " " << (i + 7) << " : " << readData[i + 7] << endl;
				i++;
			}
		}
	}
}
