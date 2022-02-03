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
	// Clear old data
	bool clearSuccessful = Clear();

	// Cycle until set to 1
	cycleTime = -1;
	while (cycleTime != 1) {
		Cycle();
	}

	// Start Measuring
	Start();
}

void BPMCommunication::Abort() {
	// Read and write from bpm here
}

bool BPMCommunication::Start()
{
    qDebug() << "BPMComm: Start";
	WriteCommand(0x11, 0x04);

	// Read output from BPM looking for a start acknolegment
	// Wait up to 30 seconds before giving up
	int result = TimedReadLoop(600,
		[this]() -> bool {
			BPMMessage msg = m_msgQueue->dequeue();
			quint8 msgId = msg.GetMsgId();
			quint8 data0 = msg.GetData0();
			if (msgId == 0x52) {
				int sbp = msg.GetData1();
				int dbp = msg.GetData2();
				int pulse = msg.GetData3();
				qDebug() << "Review (Done measuring). SBP = " << sbp << " DBP = " << dbp << "Pulse = " << pulse << endl;
				measuring = false;
				return true;
			}
			// else if Start acknowledgment...
			else if (msgId == 0x06 && data0 == 0x04) {
				qDebug() << "Ack received for start: " << msg.GetAsQString() << endl;
				cycleTime = msg.GetData1();
				readingNumber = msg.GetData2();
				measuring = true;
			}
			// else if inflating ...
			else if (msgId == 0x49) {
				cuffPressure = data0 + msg.GetData1();
				qDebug() << "Cuff pressure = " << cuffPressure << " (Inflating)" << endl;
			}
			// else if deflating ...
			else if (msgId == 0x44) {
				cuffPressure = data0 + msg.GetData1();
				qDebug() << "Cuff pressure = " << cuffPressure << " (Deflating)" << endl;
			}
			// else if bp result recieved ...
			else if (msgId == 0x42 && data0 == 0x00) {
				// TODO: Send signal with results
				int sbp = msg.GetData1();
				int dbp = msg.GetData2();
				int pulse = msg.GetData3();
				qDebug() << "Result Recieved. SBP = " << sbp << " DBP = " << dbp << "Pulse = " << pulse << endl;
			}
			// else if bp error recieved ...
			else if (msgId == 0x42 && data0 != 0x00) {
				// TODO: have an error signal to manager. Probably abort measurement
				qDebug() << "Error occured from BPM" << endl;
			}
			// else if bp average recieved ...
			else if (msgId == 0x41) {
				int sbp = msg.GetData1();
				int dbp = msg.GetData2();
				int pulse = msg.GetData3();
				qDebug() << "Average Recieved. Count = " << data0 << " SBP = " << sbp << " DBP = " << dbp << "Pulse = " << pulse << endl;
			}
			// else if review button clicked ...
			// NOTE: This gets called once by bpm even if 
			// the physical button is not clicked
			else if (msgId == 0x55 && data0 == 0x02) {
				qDebug() << "Review initiated" << endl;
			}
			else {
				qDebug() << "WARNING: Expected a start ack. But got- " << msg.GetAsQString() << endl;	
			}
			return false;
		});
	return result;
	return true;
}

bool BPMCommunication::Stop()
{
    qDebug() << "BPMComm: Stop";
    WriteCommand(0x11, 0x01);

	// Read output from BPM looking for a stop acknolegment
	// Wait up to 30 seconds before giving up
	bool stopAckRecieved = TimedReadLoop(5,
		[this]() -> bool {
			return AckCheck(1, "Stop");
		});
	// Set measuring false is stop ack sucessfully recieved
	if (stopAckRecieved) {
		measuring = false;
	}
	return stopAckRecieved;
}

bool BPMCommunication::Cycle()
{
	qDebug() << "BPMComm: Cycle";
	WriteCommand(0x11, 0x03);

	// Read output from BPM looking for a cycle acknolegment with the cycle time
	// Wait up to 30 seconds before giving up
	bool result = TimedReadLoop(5,
		[this]() -> bool {
			BPMMessage nextMessage = m_msgQueue->dequeue();
			if (nextMessage.GetMsgId() == 6 && nextMessage.GetData0() == 3) {
				cycleTime = nextMessage.GetData1();
				qDebug() << "Ack received for cycle. Cycle Time set to " << cycleTime << endl;
				return true;
			}
			else {
				qDebug() << "WARNING: Expected a cycle ack. But got- " << nextMessage.GetAsQString() << endl;
				return false;
			}
		});
	return result;
}

bool BPMCommunication::Clear()
{
	qDebug() << "BPMComm: Clear";
	WriteCommand(0x11, 0x05);

	// Read output from BPM looking for a clear acknolegment
	// Wait up to 30 seconds before giving up
	int result = TimedReadLoop(5,
		[this]() -> bool {
			return AckCheck(5, "Clear");
		});
	return result;
}

bool BPMCommunication::Review()
{
	qDebug() << "BPMComm: Review";
	WriteCommand(0x11, 0x02);

	// Read output from BPM looking for a clear acknolegment
	// Wait up to 30 seconds before giving up
	int result = TimedReadLoop(5,
		[this]() -> bool {
			BPMMessage nextMessage = m_msgQueue->dequeue();
			if (nextMessage.GetMsgId() == 6 && nextMessage.GetData0() == 2) { // Non generic condition
				qDebug() << "Ack received for review: " << nextMessage.GetAsQString() << endl;
				cycleTime = nextMessage.GetData1();
				readingNumber = nextMessage.GetData2();
				resultCode = nextMessage.GetData3();
				return true;
			}
			else {
				qDebug() << "WARNING: Expected a review ack. But got- " << nextMessage.GetAsQString() << endl;
				return false;
			}
		});
	return result;
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

/*
* Reads data from the BPM in a continuous loop and calls the passed in "func" to make
* checks on the data recieved from the BPM. The loop will continue until true is returned
* by the "func" or after "timeout" seconds. 
*/
bool BPMCommunication::TimedReadLoop(int timeout, function<bool()> func)
{
	QTime currTime = QTime::currentTime();
	QTime dieTime = currTime.addSecs(timeout);
	while (currTime < dieTime) {
		Read();
		while (m_msgQueue->isEmpty() == false) {
			bool successful = func();
			if (successful) {
				return true;
			}
		}

		currTime = QTime::currentTime();
		qDebug() << currTime << endl;
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}

	qDebug() << "Did not receive the expected data back from BPM within the time frame given" << endl;
	return false;
}

/*
* Checks if the next message in the message queue is an acknowledgement 
* for the expected command
*/
bool BPMCommunication::AckCheck(int expectedData0, QString logName)
{
	int acknowledgmentMsgId = 6;
	BPMMessage nextMessage = m_msgQueue->dequeue();
	if (nextMessage.GetMsgId() == acknowledgmentMsgId && nextMessage.GetData0() == expectedData0) {
		qDebug() << "Ack received for " << logName << endl;
		return true;
	}
	else {
		qDebug() << "WARNING: Expected a " << logName << " ack. But got- " << nextMessage.GetAsQString() << endl;
		return false;
	}
}


