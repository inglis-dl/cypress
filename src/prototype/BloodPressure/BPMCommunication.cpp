#include "BPMCommunication.h"

#include <QTime>
#include <QCoreApplication>
#include <QEventLoop>

BPMCommunication::BPMCommunication(QObject* parent) 
	: m_bpm200(new QHidDevice())
	, m_msgQueue(new QQueue<BPMMessage>())
{
}

/*
* Connect to the device
*/
void BPMCommunication::Connect(int vid, int pid) {
	m_vid = vid;
	m_pid = pid;
	bool connectionSuccessful = ConnectToBpm();
	if (connectionSuccessful) {
		// Preform handshake and update connection status
		// Assumes that bpm isn't actually connected if the handshake fails
		// NOTE: Will emit a VersionInfoAvailable signal if successful
		connectionSuccessful = Handshake();
	}
	
	emit ConnectionStatus(connectionSuccessful);
}

/*
* Preforms a meausrement using the BPM200 BPTru
* - emits the MeasurementReady signal after each bp result and after avg bp result
* - emits the MeasurementComplete signal when the review data has been received
*	and the measurement process is complete
* - emits the MeasurementFailed signal if something goes wrong
*/
void BPMCommunication::Measure() {
	qDebug() << "BPMComm: Measurement signal recieved. Preparing to measure";

	// Stop just incase it was running
	bool stopSuccessful = Stop();
	if (stopSuccessful == false) {
		emit MeasurementFailed();
		return;
	}

	// Clear old data
	bool clearSuccessful = Clear();
	qDebug() << "BPMComm: Clear data on bpm: " << (clearSuccessful ? "Successful" : "Failed");
	if (clearSuccessful == false) {
		emit MeasurementFailed();
		return;
	}

	// NOTE: At this point a stop and clear have been called successfully.
	//		 So the machine should be in a state as if just turned on

	// Cycle until set to 1
	qDebug() << "BPMComm: Cycling until set to 1";
	m_cycleTime = -1;
	bool cycleSetToOne = TimedLoop(5,
		[this]() -> bool {
			if (m_cycleTime == 1) {
				return true;
			}
			Cycle();
			return false;
		});
	if (cycleSetToOne == false) {
		emit MeasurementFailed();
		return;
	}

	// Start Measuring
	bool measurementSuccessful = Start();
	if (measurementSuccessful == false) {
		emit MeasurementFailed();
		return;
	}
}

/*
* If measuring, stop the measurement and turn off bpm.
*/
void BPMCommunication::Abort(QThread* uiThread) {
	bool connected = ConnectToBpm();
	if (connected && m_measuring) {
		Stop();
		m_aborted = true;
	}
	qDebug() << "BPM comm: abort on thread: " << QThread::currentThreadId();
	this->moveToThread(uiThread);
	qDebug() << "BPM comm: after move thread: " << QThread::currentThreadId();
	emit AbortFinished(true);
}

/*
* Attempts to connect to the bpm
* returns true if connected and false otherwise
*/
bool BPMCommunication::ConnectToBpm()
{
	// Return false if connection values not set
	if (m_vid == -1 || m_pid == -1) {
		return false;
	}

	// Check if bpm is already connected
	if (m_bpm200->isOpen()) {
		qDebug() << "Already connected";
		return true;
	}

	// Attempt to connect to bpm and return true if successful, false otherwise
	bool connectionSuccessful = m_bpm200->open(m_vid, m_pid);
	qDebug() << "BPMComm: Connection attempt with bpm: " << (connectionSuccessful ? "Successful" : "Failed");
	return connectionSuccessful;
}

/*
* Calls start on the bpm and handles communication with the bpm until one of 
* the following conditions is met:
* 1. A review message is recieved from the bpm signifying the measurement is complete
* 2. TODO: error handling
* 3. Communication with the bpm times out
* 
* If completing according to case 1 then,
* - emits the MeasurementReady signal after each bp result and after avg bp result
* - emits the MeasurementComplete signal when the review data has been received
*	and the measurement process is complete
*/
bool BPMCommunication::Start()
{
    qDebug() << "BPMComm: Start";
	bool writeCompleted = TimedWrite(0x11, 0x04);
	if (writeCompleted == false) {
		return false;
	}

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
				emit MeasurementComplete("");
				m_measuring = false;
				return true;
			}
			// else if Start acknowledgment...
			else if (msgId == 0x06 && data0 == 0x04) {
				qDebug() << "Ack received for start: " << msg.GetAsQString() << endl;
				m_cycleTime = msg.GetData1();
				m_readingNumber = msg.GetData2();
				m_measuring = true;
			}
			// else if inflating ...
			else if (msgId == 0x49) {
				m_cuffPressure = data0 + msg.GetData1();
				qDebug() << "Cuff pressure = " << m_cuffPressure << " (Inflating)" << endl;
			}
			// else if deflating ...
			else if (msgId == 0x44) {
				m_cuffPressure = data0 + msg.GetData1();
				qDebug() << "Cuff pressure = " << m_cuffPressure << " (Deflating)" << endl;
			}
			// else if bp average recieved ...
			else if (msgId == 0x41) {
				int sbp = msg.GetData1();
				int dbp = msg.GetData2();
				int pulse = msg.GetData3();
				qDebug() << "Average Recieved. Count = " << data0 << " SBP = " << sbp << " DBP = " << dbp << "Pulse = " << pulse << endl;
				emit MeasurementReady("", true);
			}
			// else if bp result recieved ...
			else if (msgId == 0x42 && data0 == 0x00) {
				// TODO: Send signal with results
				int sbp = msg.GetData1();
				int dbp = msg.GetData2();
				int pulse = msg.GetData3();
				qDebug() << "Result Recieved. SBP = " << sbp << " DBP = " << dbp << "Pulse = " << pulse << endl;
				emit MeasurementReady("", false);
			}
			// else if bp error recieved ...
			else if (msgId == 0x42 && data0 != 0x00) {
				// TODO: have an error signal to manager. Probably abort measurement
				qDebug() << "Error occured from BPM" << endl;
			}
			// else if review button clicked ...
			// NOTE: This gets called once by bpm even if 
			// the physical button is not clicked
			else if (msgId == 0x55 && data0 == 0x02) {
				qDebug() << "Review initiated" << endl;
			}
			else {
				qDebug() << "WARNING: Recieved unexpected communication while preforming start: " << msg.GetAsQString() << endl;	
			}
			return false;
		}, "Start");
	return result;
}

/*
* Calls stop on the bpm and handles communication with the bpm until one of
* the following conditions is met:
* 1. An acknowledgment of the stop request is recieved from the bpm
* 2. Communication with the bpm times out
*/
bool BPMCommunication::Stop()
{
    qDebug() << "BPMComm: Stop";
	bool writeCompleted = TimedWrite(0x11, 0x01);
	if (writeCompleted == false) {
		return false;
	}

	// Read output from BPM looking for a stop acknolegment
	// Wait up to 30 seconds before giving up
	bool stopAckRecieved = TimedReadLoop(5,
		[this]() -> bool {
			return AckCheck(1, "Stop");
		}, "Stop");
	// Set measuring false is stop ack sucessfully recieved
	if (stopAckRecieved) {
		m_measuring = false;
	}
	return stopAckRecieved;
}

/*
* Calls cycle on the bpm and handles communication with the bpm until one of
* the following conditions is met:
* 1. An acknowledgment of the cycle request is recieved from the bpm
* 2. Communication with the bpm times out
*/
bool BPMCommunication::Cycle()
{
	qDebug() << "BPMComm: Cycle";
	bool writeCompleted = TimedWrite(0x11, 0x03);
	if (writeCompleted == false) {
		return false;
	}

	// Read output from BPM looking for a cycle acknolegment with the cycle time
	// Wait up to 30 seconds before giving up
	bool result = TimedReadLoop(5,
		[this]() -> bool {
			BPMMessage nextMessage = m_msgQueue->dequeue();
			if (nextMessage.GetMsgId() == 6 && nextMessage.GetData0() == 3) {
				m_cycleTime = nextMessage.GetData1();
				qDebug() << "Ack received for cycle. Cycle Time set to " << m_cycleTime << endl;
				return true;
			}
			else {
				qDebug() << "WARNING: Expected a cycle ack. But got- " << nextMessage.GetAsQString() << endl;
				return false;
			}
		});
	return result;
}

/*
* Calls clear on the bpm and handles communication with the bpm until one of
* the following conditions is met:
* 1. An acknowledgment of the clear request is recieved from the bpm
* 2. Communication with the bpm times out
*/
bool BPMCommunication::Clear()
{
	qDebug() << "BPMComm: Clear";
	bool writeCompleted = TimedWrite(0x11, 0x05);
	if (writeCompleted == false) {
		return false;
	}

	// Read output from BPM looking for a clear acknolegment
	// Wait up to 30 seconds before giving up
	int result = TimedReadLoop(5,
		[this]() -> bool {
			return AckCheck(5, "Clear");
		});
	return result;
}

/*
* Calls review on the bpm and handles communication with the bpm until one of
* the following conditions is met:
* 1. An acknowledgment of the review request is recieved from the bpm
* 2. Communication with the bpm times out
* 
* This will also set the cycleTime, readingNumber and resultCode
*/
bool BPMCommunication::Review()
{
	qDebug() << "BPMComm: Review";
	bool writeCompleted = TimedWrite(0x11, 0x02);
	if (writeCompleted == false) {
		return false;
	}

	// Read output from BPM looking for a clear acknolegment
	// Wait up to 30 seconds before giving up
	int result = TimedReadLoop(5,
		[this]() -> bool {
			BPMMessage nextMessage = m_msgQueue->dequeue();
			if (nextMessage.GetMsgId() == 6 && nextMessage.GetData0() == 2) { // Non generic condition
				qDebug() << "Ack received for review: " << nextMessage.GetAsQString() << endl;
				m_cycleTime = nextMessage.GetData1();
				m_readingNumber = nextMessage.GetData2();
				m_resultCode = nextMessage.GetData3();
				return true;
			}
			else {
				qDebug() << "WARNING: Expected a review ack. But got- " << nextMessage.GetAsQString() << endl;
				return false;
			}
		});
	return result;
}

/*
* Initiates a handshake with the bpm and handles communication with the bpm 
* until one of the following conditions is met: 
* 1. An acknowledgment of the handshake request is recieved from the bpm
* 2. Communication with the bpm times out
*/
bool BPMCommunication::Handshake()
{
	qDebug() << "BPMComm: Handshake";
	bool writeCompleted = TimedWrite(0x11, 0x00);
	if (writeCompleted == false) {
		return false;
	}

	// Read output from BPM looking for a cycle acknolegment with the cycle time
	// Wait up to 5 seconds before giving up
	bool result = TimedReadLoop(5,
		[this]() -> bool {
			BPMMessage msg = m_msgQueue->dequeue();
			if (msg.GetMsgId() == 6 && msg.GetData0() == 0) {
				QString version = QString("%1.%2.%3").arg(msg.GetData1()).arg(msg.GetData2()).arg(msg.GetData3());
				emit VersionInfoAvailable(version);
				qDebug() << "Ack received for handshake. Version = " << version << endl;
				return true;
			}
			else {
				qDebug() << "WARNING: Expected a handshake ack. But got- " << msg.GetAsQString() << endl;
				return false;
			}
		});
	return result;
}

/*
* Writes the data passed in to the bpm if the bpm is currently connected
* If the bpm is not connected then it tries to connect for a period of time
* until it times out
*/
bool BPMCommunication::TimedWrite(quint8 msgId, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
	int timeout = 5;
	bool successful = TimedLoop(timeout,
		[this, msgId, data0, data1, data2, data3]() -> bool {
			bool connected = ConnectToBpm();
			if (connected) {
				QByteArray buf = BPMMessage::CreatePackedMessage(msgId, data0, data1, data2, data3);
				m_bpm200->write(&buf, buf.size());
				return true;
			}
			return false;
		});
	if (successful == false) {
		emit ConnectionLost();
	}
	return successful;
}

/*
* Reads data from the bpm and stores the data as BPMMessages into m_msgQueue
*/
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
* Timed read loop that calls he passed in "func". The loop will continue until true is returned
* by the "func" or after "timeout" seconds.
*/
bool BPMCommunication::TimedLoop(const int timeout, const function<bool()> func, QString debugName)
{
	QTime currTime = QTime::currentTime();
	QTime dieTime = currTime.addSecs(timeout);
	while (currTime < dieTime) {
		bool successful = func();
		if (successful) {
			return true;
		}

		currTime = QTime::currentTime();
		qDebug() << debugName << currTime << endl;
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}

	qDebug() << "Did not receive the expected data back from BPM within the time frame given" << endl;
	return false;
}

/*
* Reads data from the BPM in a continuous loop and calls the passed in "func" to make
* checks on the data recieved from the BPM. The loop will continue until true is returned
* by the "func" or after "timeout" seconds. 
*/
bool BPMCommunication::TimedReadLoop(int timeout, function<bool()> func, QString debugName)
{
	bool connected = ConnectToBpm();
	if (connected == false) {
		return false;
	}

	bool successful = TimedLoop(timeout, 
		[this, func]() -> bool {
			if (m_aborted) {
				m_aborted = false;
				return true;
			}

			Read();
			if(m_msgQueue->isEmpty() == false) {
				bool successful = func();
				if (successful) {
					return true;
				}
			}
			return false;
		}, debugName);

	return successful;
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


