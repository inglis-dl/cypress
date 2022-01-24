#include "BPMMessage.h"

BPMMessage::BPMMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    msgBytes[0] = msgID;
    msgBytes[1] = data0;
    msgBytes[2] = data1;
    msgBytes[3] = data2;
    msgBytes[4] = data3;
}

QString BPMMessage::GetAsQString()
{
    return QString("ID: %1, Data: %2, %3, %4, %5")
        .arg(QString::number(GetMsgId()),
            QString::number(GetData0()),
            QString::number(GetData1()),
            QString::number(GetData2()),
            QString::number(GetData3()));
}

QByteArray BPMMessage::PackMessage()
{
    // TODO: Improve efficency
    QByteArray packedMsg;
    packedMsg[0] = 0x00; // Report #
    packedMsg[1] = 0x02; // STX
    packedMsg[2] = GetMsgId(); // Message Id
    packedMsg[3] = GetData0(); // Data 0
    packedMsg[4] = GetData1(); // Data 1
    packedMsg[5] = GetData2(); // Data 2
    packedMsg[6] = GetData3(); // Data 3
    packedMsg[7] = CRC8::Calculate(msgBytes, 5); // CRC8
    packedMsg[8] = 0x03; // ETX
    return packedMsg;
}

QByteArray BPMMessage::CreatePackedMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    BPMMessage msg(msgID, data0, data1, data2, data3);
    return msg.PackMessage();
}

