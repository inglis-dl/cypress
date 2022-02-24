#ifndef CRC8_H
#define CRC8_H

#include <QObject>

class CRC8
{
public:
    static quint8 calculate(const QByteArray& vals, const int& length);
    static bool isValid(const QByteArray& vals, const int& length, const quint8& crc);
    static const quint8 crcLUT[];
};

#endif //CRC8_H
