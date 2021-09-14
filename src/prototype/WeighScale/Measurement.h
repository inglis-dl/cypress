#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QVariant>
#include <QDateTime>

class Measurement
{   
public:
    Measurement() = default;
    ~Measurement() = default;
    Measurement(const Measurement &);
    Measurement &operator=(const Measurement &);

    void fromArray(const QByteArray &);

    QString getUnits() const {return m_units;}
    QString getName() const {return m_name;}
    QVariant getValue() const {return m_value;}
    QString getMode() const {return m_mode;}
    QDateTime getTimestamp() const {return m_timestamp;}

    void setUnits(const QString &u){m_units=u;}
    void setName(const QString &n){m_name=n;}
    void setValue(const QVariant &v){m_value=v;}
    void setMode(const QString &m){m_mode=m;}
    void setTimestamp(const QDateTime &t){m_timestamp=t;}

    QString toString() const;
    bool isValid() const;
    bool isZero() const;
    void reset();

protected:

    QString m_units;
    QString m_name;
    QString m_mode;
    QDateTime m_timestamp;
    QVariant m_value;
};

Q_DECLARE_METATYPE(Measurement);

inline bool operator==(const Measurement &lhs, const Measurement &rhs){
    return (lhs.getUnits()==rhs.getUnits() &&
            lhs.getName()==rhs.getName() &&
            lhs.getMode()==rhs.getMode() &&
            lhs.getTimestamp()==rhs.getTimestamp() &&
            lhs.getValue()==rhs.getValue());
}
inline bool operator!=(const Measurement &lhs, const Measurement &rhs){return !(lhs == rhs);}
QDebug operator<<(QDebug dbg, const Measurement &measurement);

#endif // MEASUREMENT_H
