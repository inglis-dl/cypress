#include "BPM200.h"

#include <QCoreApplication>

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
        connect(this, &BPM200::AskForThreadId, comm, &BPMCommunication::DebugThreadId);
        connect(comm, &BPMCommunication::AbortFinished, this, &BPM200::AbortComplete);
        connect(comm, &BPMCommunication::ConnectionStatus, this, &BPM200::ReceiveConnectionStatus);
        connect(comm, &BPMCommunication::MeasurementReady, this, &BPM200::ReceiveMeasurement);
        qDebug() << "before start, Comm thread running: " << CommThread.isRunning();
        CommThread.start();
        qDebug() << "after start , Comm thread running: " << CommThread.isRunning();
    }

    emit AttemptConnection(m_vid, m_pid);
}

/*
* Attempts to disconnect from the blood pressure monitor
*/
void BPM200::Disconnect()
{
    qDebug() << "Disconnect called on thread (" << QThread::currentThread()->currentThreadId << ") , terminating thread with bpm";
    qDebug() << "Disconnect: Comm thread running: " << CommThread.isRunning();
    emit AbortMeasurement(QThread::currentThread());
    while (m_aborted == false) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    qDebug() << "Finished";
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
    qDebug() << "Disconnect called on thread: " << QThread::currentThreadId() << ", comm on thread: ";
    emit AskForThreadId();
    qDebug() << "Comm thread running: " << CommThread.isRunning();
    CommThread.quit();
    qDebug() << "Comm thread running: " << CommThread.isRunning();
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

