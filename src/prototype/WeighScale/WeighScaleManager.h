#ifndef WEIGHSCALEMANAGER_H
#define WEIGHSCALEMANAGER_H

#include <QObject>
#include <QMap>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(QSettings)

class WeighScaleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString weight MEMBER m_weight NOTIFY weightChanged)
    Q_PROPERTY(QString datetime MEMBER m_datetime NOTIFY datetimeChanged)
    Q_PROPERTY(bool verbose READ isVerbose WRITE setVerbose)

public:
    explicit WeighScaleManager(QObject *parent = nullptr);

    void loadSettings(const QSettings &);
    void saveSettings(QSettings*);

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose(){ return m_verbose; }

    const QMap<QString,QVariant>&  getMeasurementData(){return m_measurementData;}
    const QMap<QString,QVariant>&  getDeviceData(){return m_deviceData;}

    QMap<QString,QVariant> getData(){
        QMap<QString,QVariant> map(m_measurementData);
        QMap<QString,QVariant>::const_iterator it = m_deviceData.constBegin();
        while(it != m_deviceData.constEnd())
        {
          map[it.key()] = it.value();
          ++it;
        }
        return map;
    }

signals:

    void weightChanged(const QString &);
    void datetimeChanged(const QString &);

private:

    QMap<QString,QVariant> m_measurementData;
    QMap<QString,QVariant> m_deviceData;

    QString m_weight;
    QString m_datetime;
    bool m_verbose;

    void clearData();

};

#endif // WEIGHSCALEMANAGER_H
