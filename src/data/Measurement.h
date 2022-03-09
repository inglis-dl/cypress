#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QMap>
#include <QVariant>
#include <QDebug>

/*!
 * \class Measurement
 * \brief A Base Measurement class
 *
 * Measurements are composed of atomic elements or "attributes".
 * This class facilitates parsing measurement data from different
 * instruments into component characteristics, such as value, units etc.
 * and stores them labeled by QString keys as class Value objects in a QMap.
 * The measurement Value objects can be retrieved as a QString for UI purposes.
 *
 * \note  This class stores one or more attributes which taken as whole
 * can represent a single measurement.  A Test class inherited from TestBase
 * should be used to store multiple measurements.
 *
 * Child classes should override toString and isValid methods as appropriate.
 *
 * This class can also be used to store instrument attributes such as
 * serial number, model number etc. or any unitless data.
 *
 * \sa QVariant, QString, QMap
 *
 */

class Measurement
{
public:
    Measurement() = default;
    virtual ~Measurement() = default;
    Measurement(const Measurement &other)
    {
        m_attributes = other.m_attributes;
    }
    Measurement& operator=(const Measurement &other)
    {
        if(this != &other)
        {
          m_attributes = other.m_attributes;
        }
        return *this;
    }

    class Value
    {
      public:
        Value() = default;
        Value(const QVariant& value, const QString& units = QString(), const quint16& precision = 0);
        Value(const Value&);
        Value& operator=(const Value&);
        bool hasUnits() const;
        bool isNull() const;
        QString toString() const;
        QVariant value() const { return m_value; }
        QString units() const { return m_units; }
        quint16 precision() const { return m_precision; }

      private:
        QVariant m_value;
        QString m_units;

        // display digits of precision for double and float values
        //
        quint16 m_precision { 0 };
    };

    // String representation for debug and GUI display purposes
    //
    virtual QString toString() const;

    virtual QStringList toStringList() const;

    virtual bool isValid() const;

    // String keys are converted to snake_case
    //
    virtual QJsonObject toJsonObject() const;

    void reset();

    void remove(const QStringList& list)
    {
      foreach(auto key, list)
        removeAttribute(key);
    }

    void removeAttribute(const QString& key)
    {
      m_attributes.remove(key);
    }

    void setAttribute(const QString& key, const QVariant &value, const QString& units, const quint16& precision = 0)
    {
      setAttribute(key,Measurement::Value(value,units,precision));
    }

    void setAttribute(const QString& key, const QVariant &value)
    {
      setAttribute(key,Measurement::Value(value));
    }

    void setAttribute(const QString &key, const Value &value)
    {
      m_attributes[key]=value;
    }

    inline Value getAttribute(const QString &key) const
    {
        return m_attributes.contains(key) ?
               m_attributes[key] : Value();
    }

    inline QVariant getAttributeValue(const QString& key) const
    {
        return m_attributes.contains(key) ?
               m_attributes[key].value() : QVariant();
    }

    QMap<QString,Value> getAttributes() const
    {
      return m_attributes;
    }

    inline bool hasAttribute(const QString &key) const
    {
      return m_attributes.contains(key) &&
            !m_attributes[key].isNull();
    }

    inline bool hasUnits(const QString &key) const
    {
      return (hasAttribute(key) && m_attributes[key].hasUnits());
    }

    int size() const { return m_attributes.size(); }
    bool isEmpty() const { return m_attributes.isEmpty(); }

protected:
    QMap<QString,Value> m_attributes;
};

Q_DECLARE_METATYPE(Measurement);
Q_DECLARE_METATYPE(Measurement::Value)

inline bool operator==(const Measurement &lhs, const Measurement &rhs)
{
    return (lhs.getAttributes() == rhs.getAttributes());
}

inline bool operator!=(const Measurement &lhs, const Measurement &rhs)
{
    return !(lhs == rhs);
}

inline bool operator==(const Measurement::Value &lhs, const Measurement::Value &rhs)
{
    return (lhs.value() == rhs.value() && lhs.units() == rhs.units() && lhs.precision() == rhs.precision());
}

inline bool operator!=(const Measurement::Value &lhs, const Measurement::Value &rhs)
{
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const Measurement &);

#endif // MEASUREMENT_H
