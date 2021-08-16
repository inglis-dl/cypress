#include "BPMMessage.h"

BPMMessage::BPMMessage(quint8 msgID, quint8 data0)
{
    BPMMessage(msgID, data0, '0x00');
}

BPMMessage::BPMMessage(quint8 msgID, quint8 data0, quint8 data1)
{
    BPMMessage(msgID,data0,data1,'0x00', '0x00');
}

BPMMessage::BPMMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3)
{
    msgBytes.resize(6);
    msgBytes[0] = msgID;
    msgBytes[1] = data0;
    msgBytes[2] = data1;
    msgBytes[3] = data2;
    msgBytes[4] = data3;
    msgBytes[5] = CRC8::Calculate(msgBytes, 5);
}

BPMMessage::BPMMessage(quint8 msgID, quint8 data0, quint8 data1, quint8 data2, quint8 data3, quint8 crc)
{
    msgBytes.resize(6);
    msgBytes[0] = msgID;
    msgBytes[1] = data0;
    msgBytes[2] = data1;
    msgBytes[3] = data2;
    msgBytes[4] = data3;
    msgBytes[5] = crc;
}
