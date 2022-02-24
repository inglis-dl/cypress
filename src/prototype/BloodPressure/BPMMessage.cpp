#include "BPMMessage.h"

#include <QList>
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
    m_msgBytes.append(msgID);
    m_msgBytes.append(data0);
    m_msgBytes.append(data1);
    m_msgBytes.append(data2);
    m_msgBytes.append(data3);
}

QString BPMMessage::getAsQString() const
{
    return QString("ID: %1, Data: %2, %3, %4, %5")
        .arg(QString::number(getMsgId()),
            QString::number(getData0()),
            QString::number(getData1()),
            QString::number(getData2()),
            QString::number(getData3()));
}

QByteArray BPMMessage::packMessage() const
{
    QByteArray packedMsg;
    packedMsg.append(m_reportNum); // Report #
    packedMsg.append(m_stx); // STX
    packedMsg.append(getMsgId()); // Message Id
    packedMsg.append(getData0()); // Data 0
    packedMsg.append(getData1()); // Data 1
    packedMsg.append(getData2()); // Data 2
    packedMsg.append(getData3()); // Data 3
    packedMsg.append(CRC8::Calculate(m_msgBytes, 5)); // CRC8
    packedMsg.append(m_etx); // ETX
    return packedMsg;
}

QByteArray BPMMessage::createPackedMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    BPMMessage msg(msgID, data0, data1, data2, data3);
    return msg.packMessage();
}

