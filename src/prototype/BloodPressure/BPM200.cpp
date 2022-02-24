#include "BPM200.h"

#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QHidDevice>
#include <QUsbDevice>

#include "BPMMessage.h"
#include "BPMCommunication.h"

BPM200::BPM200(QObject* parent) : m_comm( new BPMCommunication()){}

/*
* Setup signal/slot connections between BPM200 and BPMCommunication
*/
void BPM200::setupConnections()
{
    // Connection bpm200 signals with comm slots
    connect(this, &BPM200::attemptConnection, m_comm, &BPMCommunication::connectToBpm);
    connect(this, &BPM200::startMeasurement, m_comm, &BPMCommunication::measure);
    connect(this, &BPM200::abortMeasurement, m_comm, &BPMCommunication::abort);

    // Connection comm signals with bpm200 slots
    connect(m_comm, &BPMCommunication::abortFinished, this, &BPM200::abortComplete);
    connect(m_comm, &BPMCommunication::connectionStatus, this, &BPM200::connectionStatusReceived);
    connect(m_comm, &BPMCommunication::measurementReady, this, &BPM200::measurementReceived);
    connect(m_comm, &BPMCommunication::averageReady, this, &BPM200::averageRecieved);
    connect(m_comm, &BPMCommunication::finalReviewReady, this, &BPM200::finalReviewRecieved);
    connect(m_comm, &BPMCommunication::measurementError, this, &BPM200::errorRecieved);

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
    if (false == connectionInfoSet()) {
        qDebug() << "Connection info has not been set";
        return;
    }

    // Setup connections if they are not already set
    if (false == m_connectionsSet) {
        setupConnections();
    }

    // Move comm to comm thread and start comm thread
    if (m_comm->thread() != &m_commThread) {
        m_comm->moveToThread(&m_commThread);
        m_commThread.start();
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
    while (false == m_aborted) {
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
    m_commThread.quit();
    m_commThread.wait();
    m_aborted = true;
}

