#include "BPMCommunication.h"

BPMCommunication::BPMCommunication(QHidDevice* bpm, QObject* parent) : m_bpm200(bpm)
{
}


void BPMCommunication::RunCommunicationLoop() {
	// Read and write from bpm here
}

void BPMCommunication::WriteCommand(quint8 msgId, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    QByteArray buf = BPMMessage::CreatePackedMessage(msgId, data0, data1, data2, data3);
    m_bpm200->write(&buf);
}