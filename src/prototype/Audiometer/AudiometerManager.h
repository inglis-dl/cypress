#ifndef AUDIOMETERMANAGER_H
#define AUDIOMETERMANAGER_H

//#include <QObject>
#include <QMap>
#include <QSerialPort>
#include <QSerialPortInfo>
//#include <QStandardItemModel>
#include <QVariant>


#include "../../managers/SerialPortManager.h"
#include "../../data/AudiometerTest.h"

QT_FORWARD_DECLARE_CLASS(QSettings)

class AudiometerManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit AudiometerManager(QObject *parent = nullptr);

    void buildModel(QStandardItemModel *) override;

public slots:

    void measure() override;

private slots:

    // retrieve data from the audiometer over RS232
    // emits canWrite signal if the test data is valid
    //
    void readDevice() override;

private:

    bool hasEndCode(const QByteArray &);

    AudiometerTest m_test;

    void clearData() override;
};

#endif // AUDIOMETERMANAGER_H
