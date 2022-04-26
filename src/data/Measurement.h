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
        Value(const QVariant& value, const quint16& precision);
        Value &operator=(const Value&);
        ~Value() = default;

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

    virtual QStringList toStringList(const bool& no_keys = false) const;

    virtual bool isValid() const;

    // String keys are converted to snake_case
    //
    virtual QJsonObject toJsonObject() const;

    void reset();

    void remove(const QStringList& list)
    {
      foreach(const auto key, list)
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

    void setAttribute(const QString& key, const QVariant &value, const quint16& precision)
    {
      setAttribute(key,Measurement::Value(value,precision));
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

    QList<QString> getAttributeKeys() const
    {
        return m_attributes.keys();
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

    // precision should be set before reading any values from the output.
    // derived classes can set precision as needed per data variable
    // ie., setting precision does not apply to all attribute values
    //
    void setPrecision(const int& precision)
    {
        if(0 < precision) m_precision = precision;
    }

    int getPrecision() const
    {
        return m_precision;
    }

protected:
    QMap<QString,Value> m_attributes;

    // display precision, actual storage precision is data intrinsic
    //
    int m_precision = { 4 };
};

Q_DECLARE_METATYPE(Measurement);
Q_DECLARE_METATYPE(Measurement::Value)

typedef Measurement MetaData;

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
