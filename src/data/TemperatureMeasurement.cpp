#include "TemperatureMeasurement.h"

#include <QDateTime>
#include <QBitArray>
#include <QDebug>

void TemperatureMeasurement::fromArray(const QByteArray &arr)
{
    if(!arr.isEmpty())
    {
    /**
     * The Temperature Measurement Value field may contain special float value NaN
     * (0x007FFFFF) defined in IEEE 11073-20601 [4] to report an invalid result from a
     * computation step or missing data due to the hardware’s inability to provide a valid
     * measurement
     *
     * flags field: 8bit
     * temperature field: sint16
     * datetime field:
     * year uint16
     * month uint8
     * day uint8
     * hours (past midnight) uint8
     * minutes (since start of hour) uint8
     * seconds (since start of minute) uint8
     * temperature type: 8bit
     *
     * example: 07 d3 03 00 ff e5 07 07 08 0f 22 00 01
     * flags: 07, hex to binary = 00000111, bit 0 = 1 => Fahrenheit, bit 1 = 1 => datetime available, bit 2 =1 => temperature type available
     * temperature: d3 03 => 03d3 = 979 => 97.9 F
     * year: e5 07 => 07 e5 => 2021
     * month: 07 => 7 => July (0 => month is not known)
     * day: 08 => 8
     * hours: 0f => 15
     * minutes: 22 => 34
     * seconds: 00 => 0
     * type: 01 => ?  body, surface or room (body)
     *
     * example: 07d80300ffe50707090b060001
     * flags: 07, hex to binary = 00000111, bit 0 = 1 => Fahrenheit, bit 1 = 1 => datetime available, bit 2 =1 => temperature type available
     * temperature: d8 03 => 03d8 = 984 => 98.4 F
     * year: e5 07 => 07 e5 => 2021
     * month: 07 => 7 => July (0 => month is not known)
     * day: 09 => 9
     * hours: 0b => 11
     * minutes: 06 => 6
     * seconds: 00 => 0
     * type: 01 => ?  body, surface or room (body)
     *
     * example: 066b0100ffe50707090c150001
     * flags: 06, hex to binary = 00000110, bit 0 = 0 => Celsius, bit 1 = 1 => datetime available, bit 2 =1 => temperature type available
     * temperature: 6b 01 => 016b = 363 => 36.3 V
     * year: e5 07 => 07 e5 => 2021
     * month: 07 => 7 => July (0 => month is not known)
     * day: 09 => 9
     * hours: 0c => 12
     * minutes: 15 => 21
     * seconds: 00 => 0
     * type: 01 => ?  body, surface or room (body)
     *
     * 06 6b 01 00 ff e5 07 07 09 0c 15 00 01
     * 0  1  2  3  4  5  6  7  8  9  10 11 12
     * f  t1 t2 x  x  y1 y2 m  d  h  j  s  t
     *
     */

     // Flags:
     // fahrenheit if bit 0 is on, celcius if off
     // datetime if bit 1 is on, no datetime if off
     // temperature type if bit 2 is on , no type if off
     //
     QBitArray f_arr = QBitArray::fromBits(arr.data(),3);
     QString t_format_str = f_arr.at(0) ? "F" : "C";
     setCharacteristic("units", t_format_str);

     // Temperature:
     //
     QByteArray t_arr = arr.mid(1,2);
     std::reverse(t_arr.begin(), t_arr.end());
     float t_value = t_arr.toHex().toInt(nullptr,16)*0.1;
     setCharacteristic("temperature", t_value);

     // Datetime:
     //
     QByteArray y_arr = arr.mid(5,2);
     std::reverse(y_arr.begin(), y_arr.end());
     int y_value = y_arr.toHex().toInt(nullptr,16);
     int m_value = arr.mid(7,1).toHex().toInt(nullptr,16);
     int d_value = arr.mid(8,1).toHex().toInt(nullptr,16);
     int h_value = arr.mid(9,1).toHex().toInt(nullptr,16);
     int j_value = arr.mid(10,1).toHex().toInt(nullptr,16);
     int s_value = arr.mid(11,1).toHex().toInt(nullptr,16);
     setCharacteristic("timestamp",
       QDateTime(
         QDate(y_value,m_value,d_value),
         QTime(h_value,j_value,s_value)
       )
     );

     // Temperature location type:
     //
     int t_type = arr.mid(12,1).toHex().toInt(nullptr,16);
     QString m_str = 1 == t_type ? "body" : "surface/room";
     setCharacteristic("mode", m_str);

     qDebug() << "received byte array: " << toString();
  }
}

TemperatureMeasurement TemperatureMeasurement::simulate()
{
   TemperatureMeasurement m;
   m.setCharacteristic("mode","body");
   m.setCharacteristic("units","C");
   m.setCharacteristic("temperature",36.1f);
   m.setCharacteristic("timestamp",QDateTime::currentDateTime());
   return m;
}

bool TemperatureMeasurement::isValid() const
{
  bool ok = false;

  if(hasCharacteristic("temperature"))
  {
    getCharacteristic("temperature").toFloat(&ok);
    ok = ok &&
    hasCharacteristic("units") &&
    hasCharacteristic("mode") &&
    hasCharacteristic("timestamp");
  }
  return ok;
}

QString TemperatureMeasurement::toString() const
{
    QString s;
    if(isValid())
    {
      QString w = QString::number(getCharacteristic("temperature").toFloat(),'f',1);
      QString u = getCharacteristic("units").toString();
      QString m = getCharacteristic("mode").toString();
      QDateTime dt = getCharacteristic("timestamp").toDateTime();
      QString d = dt.date().toString("yyyy-MM-dd");
      QString t = dt.time().toString("HH:mm:ss");
      QStringList l;
      l << w << u << m << d << t;
      s = l.join(" ");
    }
    return s;
}
