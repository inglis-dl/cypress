#include "BPMMessage.h"

BPMMessage::BPMMessage(quint8 msgID, quint8 data0)
{
    SetBytes(msgID, data0, 0x00, 0x00, 0x00);
    SetCRC();
}

BPMMessage::BPMMessage(quint8 msgID, quint8 data0, quint8 data1)
{
    SetBytes(msgID, data0, data1, 0x00, 0x00);
    SetCRC();
}

BPMMessage::BPMMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    SetBytes(msgID, data0, data1, data2, data3);
    SetCRC();
}

BPMMessage::BPMMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3, quint8 crc)
{
    SetBytes(msgID, data0, data1, data2, data3, crc);
}

void BPMMessage::SetBytes(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    msgBytes.resize(6);
    msgBytes[0] = msgID;
    msgBytes[1] = data0;
    msgBytes[2] = data1;
    msgBytes[3] = data2;
    msgBytes[4] = data3;
}

void BPMMessage::SetBytes(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3, quint8 crc)
{
    SetBytes(msgID, data0, data1, data2, data3);
    msgBytes[5] = crc;
}

void BPMMessage::SetCRC() {
    quint8 crc = CRC8::Calculate(msgBytes, 5);
    msgBytes[5] = crc;
}

QString BPMMessage::GetAsQString()
{
    return QString("ID: %1, Data: %2, %3, %4, %5, CRC: %6")
        .arg(QString::number(GetMsgId()), 
             QString::number(GetData0()), 
             QString::number(GetData1()), 
             QString::number(GetData2()), 
             QString::number(GetData3()), 
             QString::number(GetCRC()));
}
