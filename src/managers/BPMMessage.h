#ifndef BPMMESSAGE_H
#define BPMMESSAGE_H

#include <QString>
#include "../auxiliary/CRC8.h"

class BPMMessage
{
public:
    BPMMessage(const quint8& msgID,
               const quint8& data0,
               const quint8& data1 = 0x00,
               const quint8& data2 = 0x00,
               const quint8& data3 = 0x00);
    BPMMessage(const BPMMessage& other)
    {
        m_msgBytes = other.m_msgBytes;
    }
    BPMMessage& operator=(const BPMMessage& other)
    {
        if(this != &other)
        {
            m_msgBytes = other.m_msgBytes;
        }
        return *this;
    }

    bool checkCRCValid(const quint8& crcReceived)
    {
        return CRC8::isValid(m_msgBytes, 5, crcReceived);
    };

	quint8 getMsgId() const { return m_msgBytes[0]; };
	quint8 getData0() const { return m_msgBytes[1]; };
	quint8 getData1() const { return m_msgBytes[2]; };
	quint8 getData2() const { return m_msgBytes[3]; };
	quint8 getData3() const { return m_msgBytes[4]; };
	QByteArray getFullMsg() const { return m_msgBytes; };
    QString toString() const;

    static BPMMessage baseMessage()
    {
		return BPMMessage(0xff, 0xff);
	}

    static QByteArray createPackedMessage(const quint8& msgID,
                                          const quint8& data0,
                                          const quint8& data1 = 0x00,
                                          const quint8& data2 = 0x00,
                                          const quint8& data3 = 0x00);

private:
	QByteArray m_msgBytes;
    const quint8 m_reportNum { 0x00 };
    const quint8 m_stx { 0x02 };
    const quint8 m_etx { 0x03 };
    QByteArray packMessage() const;

};

#endif //BPMMESSAGE_H
