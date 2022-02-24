#include "BPMCommunication.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <QString>
#include <QHidDevice>

// NOTE: These don't seem needed anymore
//#include <functional>
//#include <iostream>

BPMCommunication::BPMCommunication(QObject* parent) 
	: m_bpm200(new QHidDevice())
	, m_msgQueue(new QQueue<BPMMessage>())
{
}

/*
* Connect to the device
*/
void BPMCommunication::connectToBpm(int vid, int pid) {
	m_vid = vid;
	m_pid = pid;
	bool connectionSuccessful = attemptConnectionToBpm();
	if (connectionSuccessful) {
		// Preform handshake and update connection status
		// Assumes that bpm isn't actually connected if the handshake fails
		// NOTE: Will emit a VersionInfoAvailable signal if successful
		connectionSuccessful = handshakeBpm();
	}
	
	emit connectionStatus(connectionSuccessful);
}

/*
* Preforms a meausrement using the BPM200 BPTru
* - emits the MeasurementReady signal after each bp result and after avg bp result
* - emits the MeasurementComplete signal when the review data has been received
*	and the measurement process is complete
* - emits the MeasurementFailed signal if something goes wrong
*/
void BPMCommunication::measure() {
	qDebug() << "BPMComm: Measurement signal recieved. Preparing to measure";
	resetValues();

	// Stop just incase it was running
	// NOTE: if already stopped then the BPM will not respond to stop request
	stopBpm();

	// Clear old data
	bool clearSuccessful = clearBpm();
	qDebug() << "BPMComm: Clear data on bpm: " << (clearSuccessful ? "Successful" : "Failed");
	if (false == clearSuccessful) {
		emit connectionStatus(false);
		return;
	}

	// NOTE: At this point a stop and clear have been called successfully.
	//		 So the machine should be in a state as if just turned on

	// Cycle until set to 1
	qDebug() << "BPMComm: Cycling until set to 1";
	m_cycleTime = -1;
	bool cycleSetToOne = timedLoop(5,
		[this]() -> bool {
			if (m_cycleTime == 1) {
				return true;
			}
			cycleBpm();
			return false;
		});
	if (false == cycleSetToOne) {
		emit connectionStatus(false);
		return;
	}

	// Start Measuring
	bool measurementSuccessful = startBpm();
	if (false == measurementSuccessful) {
		emit connectionStatus(false);
		return;
	}
}

/*
* If measuring, stop the measurement and turn off bpm.
*/
void BPMCommunication::abort(QThread* uiThread) {
	bool connected = attemptConnectionToBpm();
	if (connected && m_measuring) {
		bool stopAckRecieved = stopBpm();
		if (m_stopUnexpectedRecieved && false == stopAckRecieved) {
			// try to stop again. When error occurs, the first stop 
			// typically doesn't work, but the second does
			stopBpm();
		}
		m_aborted = true;
	}
	qDebug() << "BPM comm: abort on thread: " << QThread::currentThreadId();
	this->moveToThread(uiThread);
	qDebug() << "BPM comm: after move thread: " << QThread::currentThreadId();
	emit abortFinished(true);
}

/*
* Attempts to connect to the bpm
* returns true if connected and false otherwise
*/
bool BPMCommunication::attemptConnectionToBpm()
{
	// Return false if connection values not set
	if (-1 == m_vid || -1 == m_pid) {
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
* 2. error handling
* 3. Communication with the bpm times out
* 
* If completing according to case 1 then,
* - emits the MeasurementReady signal after each bp result and after avg bp result
* - emits the MeasurementComplete signal when the review data has been received
*	and the measurement process is complete
*/
bool BPMCommunication::startBpm()
{
    qDebug() << "BPMComm: Start";
	bool writeCompleted = timedWriteBpm(0x11, 0x04);
	if (false == writeCompleted) {
		return false;
	}

	// Read output from BPM looking for a start acknolegment
	// Wait up to 30 seconds before giving up
	int result = timedReadLoop(600,
		[this]() -> bool {
			BPMMessage msg = m_msgQueue->dequeue();
			quint8 msgId = msg.getMsgId();
			quint8 data0 = msg.getData0();
			// If review data is recieved
			if (0x52 == msgId) {
				int sbp = msg.getData1();
				int dbp = msg.getData2();
				int pulse = msg.getData3();
				bool isAverage = 0 == data0;
				qDebug() << QString("Review (%1). SBP = %2 DBP = %3 pulse = %4").arg(isAverage ? "Done measuring": "Measurement").arg(sbp).arg(dbp).arg(pulse);
				if (isAverage) {
					emit finalReviewReady(sbp, dbp, pulse);
					m_measuring = false;
					return true;
				}
			}
			// else if acknowledgment...
			else if (0x06 == msgId) {
				m_cycleTime = msg.getData1();
				if (0x04 == data0) {
					qDebug() << "Ack received for start: " << msg.getAsQString() << endl;
					m_measuring = true;
				}
				else if (0x02 == data0) {
					qDebug() << "Ack received for review: " << msg.getAsQString() << endl;
				}
			}
			// else if inflating ...
			else if (0x49 == msgId) {
				m_cuffPressure = data0 + msg.getData1();
				qDebug() << "Cuff pressure = " << m_cuffPressure << " (Inflating)" << endl;
				if (QDateTime::fromMSecsSinceEpoch(0) == m_readingStartTime) {
					startReading();
				}
			}
			// else if deflating ...
			else if (0x44 == msgId) {
				m_cuffPressure = data0 + msg.getData1();
				qDebug() << "Cuff pressure = " << m_cuffPressure << " (Deflating)" << endl;
			}
			// else if bp average recieved ...
			else if (0x41 == msgId) {
				int sbp = msg.getData1();
				int dbp = msg.getData2();
				int pulse = msg.getData3();
				qDebug() << QString("Average Recieved. Count = %1 SBP = %2 DBP = %3 pulse = %4").arg(data0).arg(sbp).arg(dbp).arg(pulse);
				emit averageReady(sbp, dbp, pulse);
			}
			// else if bp result recieved ...
			else if (0x42 == msgId && 0x00 == data0) {
				int sbp = msg.getData1();
				int dbp = msg.getData2();
				int pulse = msg.getData3();
				qDebug() << QString("Result Recieved. SBP = %1 DBP = %2 pulse = %3").arg(sbp).arg(dbp).arg(pulse);
				endReading(sbp, dbp, pulse);
			}
			// else if bp error recieved ...
			else if (0x42 == msgId && 0x00 != data0) {
				// TODO: Possibly get specific and interpret the error as a string to tell user
				emit measurementError("");
				qDebug() << "Error occured from BPM" << endl;
				return true;
			}
			// else if review button clicked ...
			// NOTE: This gets called once by bpm even if 
			// the physical button is not clicked
			else if (0x55 == msgId && 0x02 == data0) {
				qDebug() << "Review initiated" << endl;
			}
			else {
				qDebug() << "WARNING: Recieved unexpected communication while preforming start: " << msg.getAsQString() << endl;	
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
bool BPMCommunication::stopBpm()
{
	m_stopUnexpectedRecieved = false;
    qDebug() << "BPMComm: Stop";
	bool writeCompleted = timedWriteBpm(0x11, 0x01);
	if (false == writeCompleted) {
		return false;
	}

	// Read output from BPM looking for a stop acknolegment
	// Wait up to 30 seconds before giving up
	bool stopAckRecieved = timedReadLoop(5,
		[this]() -> bool {
			bool stopAckRecieved = ackCheck(1, "Stop");
			if (false == stopAckRecieved) {
				m_stopUnexpectedRecieved = true;
			}
			return stopAckRecieved;
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
bool BPMCommunication::cycleBpm()
{
	qDebug() << "BPMComm: Cycle";
	bool writeCompleted = timedWriteBpm(0x11, 0x03);
	if (writeCompleted == false) {
		return false;
	}

	// Read output from BPM looking for a cycle acknolegment with the cycle time
	// Wait up to 30 seconds before giving up
	bool result = timedReadLoop(5,
		[this]() -> bool {
			BPMMessage nextMessage = m_msgQueue->dequeue();
			if (nextMessage.getMsgId() == 6 && nextMessage.getData0() == 3) {
				m_cycleTime = nextMessage.getData1();
				qDebug() << "Ack received for cycle. Cycle Time set to " << m_cycleTime << endl;
				return true;
			}
			else {
				qDebug() << "WARNING: Expected a cycle ack. But got- " << nextMessage.getAsQString() << endl;
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
bool BPMCommunication::clearBpm()
{
	qDebug() << "BPMComm: Clear";
	bool writeCompleted = timedWriteBpm(0x11, 0x05);
	if (false == writeCompleted) {
		return false;
	}

	// Read output from BPM looking for a clear acknolegment
	// Wait up to 30 seconds before giving up
	int result = timedReadLoop(5,
		[this]() -> bool {
			return ackCheck(5, "Clear");
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
bool BPMCommunication::reviewBpm()
{
	qDebug() << "BPMComm: Review";
	bool writeCompleted = timedWriteBpm(0x11, 0x02);
	if (false == writeCompleted) {
		return false;
	}

	// Read output from BPM looking for a clear acknolegment
	// Wait up to 30 seconds before giving up
	int result = timedReadLoop(5,
		[this]() -> bool {
			BPMMessage nextMessage = m_msgQueue->dequeue();
			if (nextMessage.getMsgId() == 6 && nextMessage.getData0() == 2) { // Non generic condition
				qDebug() << "Ack received for review: " << nextMessage.getAsQString() << endl;
				m_cycleTime = nextMessage.getData1();
				//m_readingNumber = nextMessage.GetData2();
				//m_resultCode = nextMessage.GetData3();
				return true;
			}
			else {
				qDebug() << "WARNING: Expected a review ack. But got- " << nextMessage.getAsQString() << endl;
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
bool BPMCommunication::handshakeBpm()
{
	qDebug() << "BPMComm: Handshake";
	bool writeCompleted = timedWriteBpm(0x11, 0x00);
	if (writeCompleted == false) {
		return false;
	}

	// Read output from BPM looking for a cycle acknolegment with the cycle time
	// Wait up to 5 seconds before giving up
	bool result = timedReadLoop(5,
		[this]() -> bool {
			BPMMessage msg = m_msgQueue->dequeue();
			if (msg.getMsgId() == 6 && msg.getData0() == 0) {
				QString version = QString("%1.%2.%3").arg(msg.getData1()).arg(msg.getData2()).arg(msg.getData3());
				emit versionInfoAvailable(version);
				qDebug() << "Ack received for handshake. Version = " << version << endl;
				return true;
			}
			else {
				qDebug() << "WARNING: Expected a handshake ack. But got- " << msg.getAsQString() << endl;
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
bool BPMCommunication::timedWriteBpm(quint8 msgId, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
	int timeout = 5;
	bool successful = timedLoop(timeout,
		[this, msgId, data0, data1, data2, data3]() -> bool {
			bool connected = attemptConnectionToBpm();
			if (connected) {
				QByteArray buf = BPMMessage::createPackedMessage(msgId, data0, data1, data2, data3);
				m_bpm200->write(&buf, buf.size());
				return true;
			}
			return false;
		});
	if (false == successful) {
		emit connectionStatus(false);
	}
	return successful;
}

/*
* Reads data from the bpm and stores the data as BPMMessages into m_msgQueue
*/
void BPMCommunication::readFromBpm()
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
	if (0 < numBytes) {
		qDebug() << ". NumBytes: " << numBytes << endl;
		for (int i = 0; i + 7 < numBytes;) {
			quint8 firstByte = bytes[i];
			quint8 lastByte = bytes[i + 7];
			if (2 == firstByte && 3 == lastByte) {
				quint8 id = bytes[i + 1];
				quint8 data0 = bytes[i + 2];
				quint8 data1 = bytes[i + 3];
				quint8 data2 = bytes[i + 4];
				quint8 data3 = bytes[i + 5];
				quint8 crc = bytes[i + 6];
				BPMMessage msg(id, data0, data1, data2, data3);
				bool crcValid = msg.checkCRCValid(crc);
				qDebug() << "Message: " << msg.getAsQString() << endl;
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
bool BPMCommunication::timedLoop(const int timeout, const function<bool()> func, QString debugName)
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
bool BPMCommunication::timedReadLoop(int timeout, function<bool()> func, QString debugName)
{
	bool connected = attemptConnectionToBpm();
	if (connected == false) {
		return false;
	}
	m_lastBpmMessageTime = QTime::currentTime();
	bool successful = timedLoop(timeout, 
		[this, func]() -> bool {
			if (m_aborted) {
				m_aborted = false;
				return true;
			}

			readFromBpm();
			if(false == m_msgQueue->isEmpty()) {
				m_lastBpmMessageTime = QTime::currentTime();
				bool successful = func();
				if (successful) {
					return true;
				}
			}
			QTime tooLongSinceLastReadTime = m_lastBpmMessageTime.addSecs(45);
			if (QTime::currentTime() > tooLongSinceLastReadTime) {
				QString errorMessage = 
					"There has been a large pause since the last communication from the bpm. Connection status being considered interupted";
				qDebug() << errorMessage;
				emit measurementError(errorMessage);
				return true;
			}
			return false;
		}, debugName);

	return successful;
}

/*
* Checks if the next message in the message queue is an acknowledgement 
* for the expected command
*/
bool BPMCommunication::ackCheck(int expectedData0, QString logName)
{
	int acknowledgmentMsgId = 6;
	BPMMessage nextMessage = m_msgQueue->dequeue();
	if (nextMessage.getMsgId() == acknowledgmentMsgId && nextMessage.getData0() == expectedData0) {
		qDebug() << "Ack received for " << logName << endl;
		return true;
	}
	else {
		qDebug() << "WARNING: Expected a " << logName << " ack. But got- " << nextMessage.getAsQString() << endl;
		return false;
	}
}

void BPMCommunication::resetValues()
{
	m_cycleTime = -1;
	m_cuffPressure = -1;
	m_readingNumberCalc = 0;
	m_measuring = false;
	m_aborted = false;
	m_readingStartTime = QDateTime::fromMSecsSinceEpoch(0);
}

void BPMCommunication::startReading()
{
	if (QDateTime::fromMSecsSinceEpoch(0) == m_readingStartTime) {
		m_readingNumberCalc++;
		m_readingStartTime = QDateTime::currentDateTime();
	}
}

void BPMCommunication::endReading(const int& sbp, const int& dbp, const int& pulse)
{
	if (QDateTime::fromMSecsSinceEpoch(0) != m_readingStartTime) {
		emit measurementReady(sbp, dbp, pulse, m_readingStartTime, QDateTime::currentDateTime(), m_readingNumberCalc);
		m_readingStartTime = QDateTime::fromMSecsSinceEpoch(0);
	}
}


