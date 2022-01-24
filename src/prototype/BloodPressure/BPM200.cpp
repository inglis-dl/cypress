#include "BPM200.h"

BPM200::BPM200(QObject* parent) : m_bpm200(new QHidDevice())
{
}

/*
* Tell bpm to cycle
*/
void BPM200::Cycle()
{
    WriteCommand(0x11, 0x03);
}

/*
* Tell bpm to start
*/
void BPM200::Start()
{
    WriteCommand(0x11, 0x04);
}

/*
* Tell bpm to stop
*/
void BPM200::Stop()
{
    WriteCommand(0x11, 0x01);
}

/*
* Tell bpm to clear
*/
void BPM200::Clear()
{
    WriteCommand(0x11, 0x05);
}

/*
* Tell bpm to review
*/
void BPM200::Review()
{
    WriteCommand(0x11, 0x02);
}

/*
* Attempts to connect to the blood pressue monitor
* Returns true if the connection is successful or if
* the device was already connected
* and false otherwise
*/
bool BPM200::Connect()
{
    if (m_bpm200->isOpen()) {
        qDebug() << "Already connected";
        return true;
    }

    if (ConnectionInfoSet() == false) {
        qDebug() << "Connection info has not been set";
        return false;
    }

    bool successful = m_bpm200->open(m_vid, m_pid);
    if (successful) {
        qDebug() << "Successful opened a connection with the bpm";
    }
    else {
        qDebug() << "Unable to open a connection with the bpm";
    }
    return successful;
}

/*
* Attempts to disconnect from the blood pressure monitor
*/
void BPM200::Disconnect()
{
    if (m_bpm200->isOpen()) {
        qDebug() << "Closing connection with bpm";
        m_bpm200->close();
    }
    else {
        qDebug() << "Cannot disconnect: Connection with bpm is not open";
    }
}

void BPM200::WriteCommand(quint8 msgId, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    QByteArray buf = BPMMessage::CreatePackedMessage(msgId, data0, data1, data2, data3);
    m_bpm200->write(&buf);
}

/*
* True if the vid and pid values have been set
* and false otherwise
*/
bool BPM200::ConnectionInfoSet()
{
    return m_vid > 0 && m_pid > 0;
}
