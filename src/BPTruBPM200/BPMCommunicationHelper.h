#pragma once

#include <QQueue>
#include <QCloseEvent>
#include <QTime>

#include "BPMMessage.h"
#include "CommunicationThread.h"
#include "BPMCommands.h"

class BPMCommunicationHelper
{
public:
    bool StartBPMCommunication() {
        cThread.SetValues(&writeQueue, &readQueue, &stopCommunicationThread);
        cThread.start();
        return true;
    }
    bool StopBPMCommunication() {
        qDebug() << "Close event called at " << QTime::currentTime() << endl;

        // Stop comunication thread thread and 
        // wait for it to stop before closing app
        stopCommunicationThread = true;
        while (cThread.isRunning()) {}

        qDebug() << "Closing at " << QTime::currentTime() << endl;
        return true;
    }
    void AddToWriteQueue(BPMMessage message) {
        writeQueue.append(message);
    }
    BPMMessage ReadFromReadQueue() {
        if (readQueue.count() > 0) {
            BPMMessage msg = readQueue.dequeue();
            return msg;
        }
        return BPMMessage::BaseMessage();
    }
private:
    CommunicationThread cThread;
    bool stopCommunicationThread = false;
    QQueue<BPMMessage> writeQueue;
    QQueue<BPMMessage> readQueue;
};

