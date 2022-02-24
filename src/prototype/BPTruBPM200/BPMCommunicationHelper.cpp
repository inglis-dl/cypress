#include "BPMCommunicationHelper.h"

/*
* Starts the BPM Communication thread to allow for communication between
* this app and the BPM
* isLive: True if this is being run live and should interact with BPM
*         False if in Test mode and should interact with a mock test BPM
*/
bool BPMCommunicationHelper::StartBPMCommunication(bool isLive) {
    if (cThread.isRunning() == false) {
        cThread.SetValues(&writeQueue, &readQueue, &stopCommunicationThread, isLive);
        cThread.start();
    }
    return true;
}

/*
* Stops the BPM Communication thread which stops communication between
* this app and the BPM
* NOTE: This needs to be called before exiting this app
*/
bool BPMCommunicationHelper::StopBPMCommunication() {
    qDebug() << "Close event called at " << QTime::currentTime() << endl;

    // Stop comunication thread thread and 
    // wait for it to stop before closing app
    stopCommunicationThread = true;
    while (cThread.isRunning()) {}

    qDebug() << "Closing at " << QTime::currentTime() << endl;
    return true;
}

/*
* Write a message to be sent to the BPM
*/
void BPMCommunicationHelper::Write(BPMMessage message) { 
    writeQueue.append(message); 
}

/*
* Read the first available message that has been received from the BPM
*/
BPMMessage BPMCommunicationHelper::Read() {
    if (MessagesAvailable()) {
        BPMMessage msg = readQueue.dequeue();
        return msg;
    }
    return BPMMessage::BaseMessage();
}

/*
* Check if there are any messages available from the BPM
*/
bool BPMCommunicationHelper::MessagesAvailable() { 
    return readQueue.count() > 0; 
}
