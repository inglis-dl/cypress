#include "BPMCommunication.h"

#include "BPMMessage.h"
#include "../auxiliary/Utilities.h"

#include <QCoreApplication>
#include <QHidDevice>
#include <QThread>

BPMCommunication::BPMCommunication(QObject* parent) : QObject(parent)
    , m_bpm200(new QHidDevice(this))
    , m_queue(new QQueue<BPMMessage>())
{
}

BPMCommunication::~BPMCommunication()
{
    delete m_bpm200;
    delete m_queue;
}

/*
* Connect to the device
*/
void BPMCommunication::connect(const QUsb::Id& info)
{
    m_info = info;
	bool connectionSuccessful = attemptConnectionToBpm();
    if(connectionSuccessful)
    {
		// Preform handshake and update connection status
		// Assumes that bpm isn't actually connected if the handshake fails
        // NOTE: Will emit a deviceInfoReady signal if successful
        //
		connectionSuccessful = handshakeBpm();
	}
	
	emit connectionStatus(connectionSuccessful);
}

void BPMCommunication::disconnect()
{
    if(m_bpm200->isOpen())
    {
        m_bpm200->close();
    }
}

/*
* Preforms a meausrement using the BPM200 BPTru
* - emits the MeasurementReady signal after each bp result and after avg bp result
* - emits the MeasurementComplete signal when the review data has been received
*	and the measurement process is complete
* - emits the MeasurementFailed signal if something goes wrong
*/
void BPMCommunication::measure()
{
	qDebug() << "BPMComm: Measurement signal received. Preparing to measure";
	resetValues();

    // Stop just in case it was running
	// NOTE: if already stopped then the BPM will not respond to stop request
	stopBpm();

	// Clear old data
	bool clearSuccessful = clearBpm();
	qDebug() << "BPMComm: Clear data on bpm: " << (clearSuccessful ? "Successful" : "Failed");
    if(!clearSuccessful)
    {
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
            if(1 == m_cycleTime)
            {
				return true;
			}
			cycleBpm();
			return false;
		});
    if(!cycleSetToOne)
    {
		emit connectionStatus(false);
		return;
	}

	// Start Measuring
	bool measurementSuccessful = startBpm();
    if(!measurementSuccessful)
    {
		emit connectionStatus(false);
		return;
	}
}

/*
* If measuring, stop the measurement and turn off bpm.
*/
void BPMCommunication::abort(QThread* uiThread)
{
	bool connected = attemptConnectionToBpm();
    if(connected && m_measuring)
    {
		bool stopAckReceived = stopBpm();
        if(m_stopUnexpectedReceived && !stopAckReceived)
        {
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
    if(0 == m_info.pid || 0 == m_info.vid)
    {
      return false;
	}

	// Check if bpm is already connected
    if(m_bpm200->isOpen())
    {
      qDebug() << "Already connected";
      return true;
	}

	// Attempt to connect to bpm and return true if successful, false otherwise
    bool connectionSuccessful = m_bpm200->open(m_info.vid, m_info.pid);
	qDebug() << "BPMComm: Connection attempt with bpm: " << (connectionSuccessful ? "Successful" : "Failed");
	return connectionSuccessful;
}

/*
* Calls start on the bpm and handles communication with the bpm until one of 
* the following conditions is met:
* 1. A review message is received from the bpm signifying the measurement is complete
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
    if(!writeCompleted)
    {
		return false;
	}

    // Read output from BPM looking for a start acknowledgment
	// Wait up to 30 seconds before giving up
    bool result = timedReadLoop(600,
		[this]() -> bool {
            BPMMessage msg = m_queue->dequeue();
			quint8 msgId = msg.getMsgId();
			quint8 data0 = msg.getData0();
			// If review data is received
            if(0x52 == msgId)
            {
				int sbp = msg.getData1();
				int dbp = msg.getData2();
				int pulse = msg.getData3();
				bool isAverage = 0 == data0;
				qDebug() << QString("Review (%1). SBP = %2 DBP = %3 pulse = %4").arg(isAverage ? "Done measuring": "Measurement").arg(sbp).arg(dbp).arg(pulse);
                if(isAverage)
                {
					emit finalReviewReady(sbp, dbp, pulse);
					m_measuring = false;
					return true;
				}
			}
            // else if acknowledgment...
            else if(0x06 == msgId)
            {
				m_cycleTime = msg.getData1();
                if(0x04 == data0)
                {
                    qDebug() << "Ack received for start: " << msg.toString();
					m_measuring = true;
				}
                else if(0x02 == data0)
                {
                    qDebug() << "Ack received for review: " << msg.toString();
				}
			}
			// else if inflating ...
            else if(0x49 == msgId)
            {
				m_cuffPressure = data0 + msg.getData1();
                qDebug() << "Cuff pressure = " << m_cuffPressure << " (Inflating)";
                if(QDateTime::fromMSecsSinceEpoch(0) == m_readingStartTime)
                {
					startReading();
				}
			}
			// else if deflating ...
            else if(0x44 == msgId)
            {
				m_cuffPressure = data0 + msg.getData1();
                qDebug() << "Cuff pressure = " << m_cuffPressure << " (Deflating)";
			}
			// else if bp average received ...
            else if(0x41 == msgId)
            {
				int sbp = msg.getData1();
				int dbp = msg.getData2();
				int pulse = msg.getData3();
				qDebug() << QString("Average Received. Count = %1 SBP = %2 DBP = %3 pulse = %4").arg(data0).arg(sbp).arg(dbp).arg(pulse);
				emit averageReady(sbp, dbp, pulse);
			}
			// else if bp result received ...
            else if(0x42 == msgId && 0x00 == data0)
            {
				int sbp = msg.getData1();
				int dbp = msg.getData2();
				int pulse = msg.getData3();
				qDebug() << QString("Result Received. SBP = %1 DBP = %2 pulse = %3").arg(sbp).arg(dbp).arg(pulse);
				endReading(sbp, dbp, pulse);
			}
			// else if bp error received ...
            else if(0x42 == msgId && 0x00 != data0)
            {
                // TODO: Possibly get specific and interpret the error as a string to tell user
				emit measurementError("");
                qDebug() << "Error occured from BPM";
				return true;
			}
			// else if review button clicked ...
			// NOTE: This gets called once by bpm even if 
			// the physical button is not clicked
            else if(0x55 == msgId && 0x02 == data0)
            {
                qDebug() << "Review initiated";
			}
            else
            {
                qDebug() << "WARNING: Received unexpected communication while preforming start: " << msg.toString();
			}
			return false;
		}, "Start");
	return result;
}

/*
* Calls stop on the bpm and handles communication with the bpm until one of
* the following conditions is met:
* 1. An acknowledgment of the stop request is received from the bpm
* 2. Communication with the bpm times out
*/
bool BPMCommunication::stopBpm()
{
    m_stopUnexpectedReceived = false;
    qDebug() << "BPMComm: Stop";
	bool writeCompleted = timedWriteBpm(0x11, 0x01);
    if(!writeCompleted)
    {
		return false;
	}

	// Read output from BPM looking for a stop acknolegment
	// Wait up to 30 seconds before giving up
	bool stopAckReceived = timedReadLoop(5,
		[this]() -> bool {
			bool stopAckReceived = ackCheck(1, "Stop");
            if(!stopAckReceived)
            {
                m_stopUnexpectedReceived = true;
			}
			return stopAckReceived;
		}, "Stop");
	// Set measuring false is stop ack sucessfully received
    if(stopAckReceived)
    {
		m_measuring = false;
	}
	return stopAckReceived;
}

/*
* Calls cycle on the bpm and handles communication with the bpm until one of
* the following conditions is met:
* 1. An acknowledgment of the cycle request is received from the bpm
* 2. Communication with the bpm times out
*/
bool BPMCommunication::cycleBpm()
{
	qDebug() << "BPMComm: Cycle";
	bool writeCompleted = timedWriteBpm(0x11, 0x03);
    if(!writeCompleted)
    {
		return false;
	}

    // Read output from BPM looking for a cycle acknowledgment with the cycle time
	// Wait up to 30 seconds before giving up
	bool result = timedReadLoop(5,
		[this]() -> bool {
            BPMMessage nextMessage = m_queue->dequeue();
            if(6 == nextMessage.getMsgId() && 3 == nextMessage.getData0())
            {
				m_cycleTime = nextMessage.getData1();
                qDebug() << "Ack received for cycle. Cycle Time set to " << m_cycleTime;
				return true;
			}
            else
            {
                qDebug() << "WARNING: Expected a cycle ack. But got- " << nextMessage.toString();
				return false;
			}
		});
	return result;
}

/*
* Calls clear on the bpm and handles communication with the bpm until one of
* the following conditions is met:
* 1. An acknowledgment of the clear request is received from the bpm
* 2. Communication with the bpm times out
*/
bool BPMCommunication::clearBpm()
{
	qDebug() << "BPMComm: Clear";
	bool writeCompleted = timedWriteBpm(0x11, 0x05);
    if(!writeCompleted)
    {
		return false;
	}

    // Read output from BPM looking for a clear acknowledgment
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
* 1. An acknowledgment of the review request is received from the bpm
* 2. Communication with the bpm times out
* 
* This will also set the cycleTime, readingNumber and resultCode
*/
bool BPMCommunication::reviewBpm()
{
	qDebug() << "BPMComm: Review";
	bool writeCompleted = timedWriteBpm(0x11, 0x02);
    if(!writeCompleted)
    {
		return false;
	}

    // Read output from BPM looking for a clear acknowledgment
	// Wait up to 30 seconds before giving up
	int result = timedReadLoop(5,
		[this]() -> bool {
            BPMMessage nextMessage = m_queue->dequeue();
            if(nextMessage.getMsgId() == 6 && nextMessage.getData0() == 2)
            { // Non generic condition
                qDebug() << "Ack received for review: " << nextMessage.toString();
				m_cycleTime = nextMessage.getData1();
				//m_readingNumber = nextMessage.GetData2();
				//m_resultCode = nextMessage.GetData3();
				return true;
			}
			else {
                qDebug() << "WARNING: Expected a review ack but received " << nextMessage.toString();
				return false;
			}
		});
	return result;
}

/*
* Initiates a handshake with the bpm and handles communication with the bpm 
* until one of the following conditions is met: 
* 1. An acknowledgment of the handshake request is received from the bpm
* 2. Communication with the bpm times out
*/
bool BPMCommunication::handshakeBpm()
{
	qDebug() << "BPMComm: Handshake";
	bool writeCompleted = timedWriteBpm(0x11, 0x00);
    if(!writeCompleted)
    {
      return false;
	}

    // Read output from BPM looking for a cycle acknowledgment with the cycle time
	// Wait up to 5 seconds before giving up
	bool result = timedReadLoop(5,
		[this]() -> bool {
            BPMMessage msg = m_queue->dequeue();
            if(6 == msg.getMsgId() && 0 == msg.getData0())
            {
                m_version = QString("%1.%2.%3").arg(msg.getData1()).arg(msg.getData2()).arg(msg.getData3());
                m_product = m_bpm200->product();
                m_serialNumber = m_bpm200->serialNumber();
                m_manufacturer = m_bpm200->manufacturer();
                emit deviceInfoReady();
                qDebug() << "Ack received for handshake. Version = " << m_version;
				return true;
			}
            else
            {
                qDebug() << "WARNING: Expected a handshake ack but received " << msg.toString();
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
bool BPMCommunication::timedWriteBpm(
        const quint8& msgId, const quint8& data0,
        const quint8& data1, const quint8& data2, const quint8& data3)
{
	int timeout = 5;
	bool successful = timedLoop(timeout,
		[this, msgId, data0, data1, data2, data3]() -> bool {
			bool connected = attemptConnectionToBpm();
            if(connected)
            {
				QByteArray buf = BPMMessage::createPackedMessage(msgId, data0, data1, data2, data3);
				m_bpm200->write(&buf, buf.size());
				return true;
			}
			return false;
		});
    if(!successful)
    {
		emit connectionStatus(false);
	}
	return successful;
}

/*
* Reads data from the bpm and stores the data as BPMMessages into m_queue
*/
void BPMCommunication::readFromBpm()
{
	// Read from the BPM
	const int dataLength = 64;
	QByteArray readData[dataLength];
	int numBytesReturned = m_bpm200->read(readData, dataLength, 300);

	// Stop here if no data was read in
    if(0 >= numBytesReturned) return;

	// Convert read in QByteArray to a QByteArray with values that can be used as quint8
	// NOTE: Typically all the data is stored in index 0 of readData which is why the 
	//		 toHex then FromHex needs to be done. However this code does not assume that 
	//		 will always be the case. It still checks each byte in readData up to index 
    //       numBytesReturned - 1 just in case their is data in any of those indices
    //
	QByteArray bytes = QByteArray::fromHex(readData[0].toHex());
    for(int i = 1; i < numBytesReturned; i++)
    {
		QByteArray tempBytes = QByteArray::fromHex(readData[i].toHex());
		bytes.append(tempBytes);
	}

	// Convert bytes to BPMMessages
	int numBytes = bytes.size();
    if(0 < numBytes)
    {
        qDebug() << "NumBytes:" << numBytes;
        for(int i = 0; i + 7 < numBytes;)
        {
			quint8 firstByte = bytes[i];
			quint8 lastByte = bytes[i + 7];
            if(2 == firstByte && 3 == lastByte)
            {
				quint8 id = bytes[i + 1];
				quint8 data0 = bytes[i + 2];
				quint8 data1 = bytes[i + 3];
				quint8 data2 = bytes[i + 4];
				quint8 data3 = bytes[i + 5];
				quint8 crc = bytes[i + 6];
				BPMMessage msg(id, data0, data1, data2, data3);

                bool crcValid = Utilities::checksumIsValid(msg.getFullMsg(), 5, crc);
                qDebug() << "Message:" << msg.toString();
                qDebug() << "CRC Valid:" << crcValid;
                if(crcValid)
                {
                    m_queue->enqueue(msg);
				}
				i += 8;
			}
            else
            {
                qDebug() << i << " : " << readData[i] << " " << (i + 7) << " : " << readData[i + 7];
				i++;
			}
		}
	}
}

/*
* Timed read loop that calls he passed in "func". The loop will continue until true is returned
* by the "func" or after "timeout" seconds.
*/
bool BPMCommunication::timedLoop(const int& timeout,
  const std::function<bool()>& func, const QString& debugName)
{
	QTime currTime = QTime::currentTime();
	QTime dieTime = currTime.addSecs(timeout);
    while(currTime < dieTime)
    {
		bool successful = func();
        if(successful)
        {
			return true;
		}

		currTime = QTime::currentTime();
        qDebug() << debugName << currTime;
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}

    qDebug() << "Did not receive the expected data back from BPM within the time frame given";
	return false;
}

/*
* Reads data from the BPM in a continuous loop and calls the passed in "func" to make
* checks on the data received from the BPM. The loop will continue until true is returned
* by the "func" or after "timeout" seconds. 
*/
bool BPMCommunication::timedReadLoop(const int& timeout,
  const std::function<bool()>& func, const QString& debugName)
{
	bool connected = attemptConnectionToBpm();
    if(!connected)
    {
		return false;
	}
	m_lastBpmMessageTime = QTime::currentTime();
	bool successful = timedLoop(timeout, 
		[this, func]() -> bool {
            if(m_aborted)
            {
				m_aborted = false;
				return true;
			}

			readFromBpm();
            if(!m_queue->isEmpty())
            {
				m_lastBpmMessageTime = QTime::currentTime();
				bool successful = func();
                if(successful)
                {
					return true;
				}
			}
			QTime tooLongSinceLastReadTime = m_lastBpmMessageTime.addSecs(45);
            if(QTime::currentTime() > tooLongSinceLastReadTime)
            {
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
* Checks if the next message in the message queue is an acknowledgment
* for the expected command
*/
bool BPMCommunication::ackCheck(const int& expectedData0, const QString& logName)
{
	int acknowledgmentMsgId = 6;
    BPMMessage nextMessage = m_queue->dequeue();
    if(nextMessage.getMsgId() == acknowledgmentMsgId &&
       nextMessage.getData0() == expectedData0)
    {
        qDebug() << "Ack received for " << logName;
		return true;
	}
    else
    {
        qDebug() << "WARNING: Expected a " << logName << " ack but received " << nextMessage.toString();
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
    if(QDateTime::fromMSecsSinceEpoch(0) == m_readingStartTime)
    {
		m_readingNumberCalc++;
		m_readingStartTime = QDateTime::currentDateTime();
	}
}

void BPMCommunication::endReading(const int& sbp, const int& dbp, const int& pulse)
{
    if(QDateTime::fromMSecsSinceEpoch(0) != m_readingStartTime)
    {
        emit measurementReady(m_readingNumberCalc, sbp, dbp, pulse, m_readingStartTime, QDateTime::currentDateTime());
		m_readingStartTime = QDateTime::fromMSecsSinceEpoch(0);
	}
}
