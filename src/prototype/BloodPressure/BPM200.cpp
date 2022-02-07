#include "BPM200.h"

BPM200::BPM200(QObject* parent) : comm( new BPMCommunication()){}

/*
* Attempts to connect to the blood pressue monitor
* Returns true if the connection is successful or if
* the device was already connected
* and false otherwise
*/
void BPM200::Connect()
{
    if (ConnectionInfoSet() == false) {
        qDebug() << "Connection info has not been set";
        return;
    }

    // Move to thread and setup connection if comm is not on comm thread
    if (comm->thread() != &CommThread) {
        comm->moveToThread(&CommThread);
        connect(this, &BPM200::AttemptConnection, comm, &BPMCommunication::Connect);
        connect(this, &BPM200::StartMeasurement, comm, &BPMCommunication::Measure);
        connect(this, &BPM200::AbortMeasurement, comm, &BPMCommunication::Abort);
        connect(comm, &BPMCommunication::AbortFinished, this, &BPM200::AbortComplete);
        connect(comm, &BPMCommunication::ConnectionStatus, this, &BPM200::ReceiveConnectionStatus);
        connect(comm, &BPMCommunication::MeasurementReady, this, &BPM200::ReceiveMeasurement);
        CommThread.start();
    }

    emit AttemptConnection(m_vid, m_pid);
}

/*
* Attempts to disconnect from the blood pressure monitor
*/
void BPM200::Disconnect()
{
    qDebug() << "Disconnect called, terminating thread with bpm";
    emit AbortMeasurement();
}

void BPM200::ReceiveConnectionStatus(bool connected) {
    if (connected) {
        // TODO: Change later, this is here just for testing
        emit StartMeasurement();
    }
}

void BPM200::ReceiveMeasurement(QString measurement) {

}

void BPM200::AbortComplete(bool successful) {
    CommThread.quit();
    CommThread.wait();
}

/*
* True if the vid and pid values have been set
* and false otherwise
*/
bool BPM200::ConnectionInfoSet()
{
    return m_vid > 0 && m_pid > 0;
}

