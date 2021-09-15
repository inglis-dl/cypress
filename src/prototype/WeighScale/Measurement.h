#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QVariant>
#include <QMap>

class Measurement
{   
public:
    Measurement() = default;
    ~Measurement() = default;
    Measurement(const Measurement &);
    Measurement &operator=(const Measurement &);

    void fromArray(const QByteArray &);

    QString toString() const;
    bool isValid() const;
    bool isZero() const;
    void reset();
    void setCharacteristic(const QString &key, const QVariant &value)
    {
      m_characteristicValues[key]=value;
    }

    QVariant getCharacteristic(const QString &key) const
    {
        return m_characteristicValues.contains(key) ?
               m_characteristicValues[key] : QVariant();
    }

    QMap<QString,QVariant> getCharacteristicValues() const
    {
        return m_characteristicValues;
    }

protected:

    QMap<QString,QVariant> m_characteristicValues;
};

Q_DECLARE_METATYPE(Measurement);

inline bool operator==(const Measurement &lhs, const Measurement &rhs){
    return  lhs.getCharacteristicValues()==rhs.getCharacteristicValues();
}
inline bool operator!=(const Measurement &lhs, const Measurement &rhs){return !(lhs == rhs);}
QDebug operator<<(QDebug dbg, const Measurement &measurement);

#endif // MEASUREMENT_H
