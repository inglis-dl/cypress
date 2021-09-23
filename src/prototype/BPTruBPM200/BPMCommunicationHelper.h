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
    bool StartBPMCommunication(bool isLive);
    bool StopBPMCommunication();
    void Write(BPMMessage message);
    BPMMessage Read();
    bool MessagesAvailable();
    CommunicationThread cThread;
private:
    bool stopCommunicationThread = false;
    QQueue<BPMMessage> writeQueue;
    QQueue<BPMMessage> readQueue;
};

