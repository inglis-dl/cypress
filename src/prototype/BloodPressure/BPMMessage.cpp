#include "BPMMessage.h"

/*
* NOTE: The previous version of this constructor caused the following warning:
     Using QByteRef with an index pointing outside the valid range of a QByteArray. 
     The corresponding behavior is deprecated, and will be changed in a future version of Qt.

     CODE:
        msgBytes[0] = msgID;
        msgBytes[1] = data0;
        msgBytes[2] = data1;
        msgBytes[3] = data2;
        msgBytes[4] = data3;
*/
BPMMessage::BPMMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    msgBytes.append(msgID);
    msgBytes.append(data0);
    msgBytes.append(data1);
    msgBytes.append(data2);
    msgBytes.append(data3);
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
    QByteArray packedMsg;
    packedMsg.append(reportNum); // Report #
    packedMsg.append(stx); // STX
    packedMsg.append(GetMsgId()); // Message Id
    packedMsg.append(GetData0()); // Data 0
    packedMsg.append(GetData1()); // Data 1
    packedMsg.append(GetData2()); // Data 2
    packedMsg.append(GetData3()); // Data 3
    packedMsg.append(CRC8::Calculate(msgBytes, 5)); // CRC8
    packedMsg.append(etx); // ETX
    return packedMsg;
}

QByteArray BPMMessage::CreatePackedMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    BPMMessage msg(msgID, data0, data1, data2, data3);
    return msg.PackMessage();
}

