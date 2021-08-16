#pragma once

#include <QDebug>

class CRC8
{
public:
	static quint8 Calculate(QByteArray vals, int length);
	static bool ValidCrc(QByteArray vals, int length, quint8 crc);
	static const quint8 lookupTable[];
};

