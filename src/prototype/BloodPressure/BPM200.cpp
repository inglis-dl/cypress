#include "BPM200.h"

#include <QCoreApplication>

BPM200::BPM200(QObject* parent) : comm( new BPMCommunication()){}

/*
* Setup signal/slot connections between BPM200 and BPMCommunication
*/
void BPM200::SetupConnections()
{
    // Connection bpm200 signals with comm slots
    connect(this, &BPM200::AttemptConnection, comm, &BPMCommunication::Connect);
    connect(this, &BPM200::StartMeasurement, comm, &BPMCommunication::Measure);
    connect(this, &BPM200::AbortMeasurement, comm, &BPMCommunication::Abort);

    // Connection comm signals with bpm200 slots
    connect(comm, &BPMCommunication::AbortFinished, this, &BPM200::AbortComplete);
    connect(comm, &BPMCommunication::ConnectionStatus, this, &BPM200::ConnectionStatusReceived);
    connect(comm, &BPMCommunication::MeasurementReady, this, &BPM200::MeasurementReceived);

    // Set connections set to true
    m_connectionsSet = true;
}

/*
* Attempts to connect to the blood pressue monitor
* Returns true if the connection is successful or if
* the device was already connected
* and false otherwise
*/
void BPM200::Connect()
{
    // Do not attempt connection unless the required 
    // connection info has not been set
    if (ConnectionInfoSet() == false) {
        qDebug() << "Connection info has not been set";
        return;
    }

    // Setup connections if they are not already set
    if (m_connectionsSet == false) {
        SetupConnections();
    }

    // Move comm to comm thread and start comm thread
    if (comm->thread() != &CommThread) {
        comm->moveToThread(&CommThread);
        CommThread.start();
    }

    // Attempt to connect to bpm
    emit AttemptConnection(m_vid, m_pid);
}

/*
* Attempts to disconnect from the blood pressure monitor
*/
void BPM200::Disconnect()
{
    qDebug() << "BPM200: Disconnect called, terminating thread with bpm";
    emit AbortMeasurement(QThread::currentThread());
    while (m_aborted == false) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    qDebug() << "BPM200: Finished";
}

/*
* This gets called once the abort has completed on the comm thread
* Then this will close the comm thread and set a bool to notify 
* that the abort has completed
*/
void BPM200::AbortComplete(bool successful) {
    qDebug() << "BPM200: Abort complete";
    CommThread.quit();
    CommThread.wait();
    m_aborted = true;
}

/*
* True if the vid and pid values have been set
* and false otherwise
*/
bool BPM200::ConnectionInfoSet()
{
    return m_vid > 0 && m_pid > 0;
}

