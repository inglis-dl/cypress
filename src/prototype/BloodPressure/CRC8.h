#ifndef CRC8_H
#define CRC8_H

#include <QDebug>

class CRC8
{
public:
	static quint8 Calculate(QByteArray vals, int length);
	static bool ValidCrc(QByteArray vals, int length, quint8 crc);
	static const quint8 lookupTable[];
};

#endif //CRC8_H