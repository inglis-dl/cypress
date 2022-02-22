#include "BPM200.h"

#include <QCoreApplication>

BPM200::BPM200(QObject* parent) : comm( new BPMCommunication()){}

/*
* Setup signal/slot connections between BPM200 and BPMCommunication
*/
void BPM200::setupConnections()
{
    // Connection bpm200 signals with comm slots
    connect(this, &BPM200::attemptConnection, comm, &BPMCommunication::connectToBpm);
    connect(this, &BPM200::startMeasurement, comm, &BPMCommunication::measure);
    connect(this, &BPM200::abortMeasurement, comm, &BPMCommunication::abort);

    // Connection comm signals with bpm200 slots
    connect(comm, &BPMCommunication::abortFinished, this, &BPM200::abortComplete);
    connect(comm, &BPMCommunication::connectionStatus, this, &BPM200::connectionStatusReceived);
    connect(comm, &BPMCommunication::measurementReady, this, &BPM200::measurementReceived);
    connect(comm, &BPMCommunication::averageReady, this, &BPM200::averageRecieved);
    connect(comm, &BPMCommunication::finalReviewReady, this, &BPM200::finalReviewRecieved);
    connect(comm, &BPMCommunication::measurementError, this, &BPM200::errorRecieved);

    // Set connections set to true
    m_connectionsSet = true;
}

/*
* Attempts to connect to the blood pressue monitor
* Returns true if the connection is successful or if
* the device was already connected
* and false otherwise
*/
void BPM200::connectToBpm()
{
    // Do not attempt connection unless the required 
    // connection info has not been set
    if (connectionInfoSet() == false) {
        qDebug() << "Connection info has not been set";
        return;
    }

    // Setup connections if they are not already set
    if (m_connectionsSet == false) {
        setupConnections();
    }

    // Move comm to comm thread and start comm thread
    if (comm->thread() != &CommThread) {
        comm->moveToThread(&CommThread);
        CommThread.start();
    }

    // Attempt to connect to bpm
    emit attemptConnection(m_vid, m_pid);
}

/*
* Attempts to disconnect from the blood pressure monitor
*/
void BPM200::disconnect()
{
    qDebug() << "BPM200: Disconnect called, terminating thread with bpm";
    emit abortMeasurement(QThread::currentThread());
    while (m_aborted == false) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    qDebug() << "BPM200: Finished";
}

QList<int> BPM200::findAllPids()
{
    QUsb::IdList usbDeviceList = m_usb.devices();
    
    QList<int> matchingPids;
    for each (auto entry in usbDeviceList)
    {
        qDebug() << entry;
        if (entry.vid == m_vid) {
            int pid = entry.pid;
            qDebug() << "found one with pid = " << pid;
            if (matchingPids.contains(pid) == false) {
                matchingPids.append(pid);
            }
        }
    }
    return matchingPids;
}

/*
* This gets called once the abort has completed on the comm thread
* Then this will close the comm thread and set a bool to notify 
* that the abort has completed
*/
void BPM200::abortComplete(bool successful) {
    qDebug() << "BPM200: Abort complete";
    CommThread.quit();
    CommThread.wait();
    m_aborted = true;
}

