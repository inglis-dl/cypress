#ifndef WEIGHSCALEMANAGER_H
#define WEIGHSCALEMANAGER_H

#include "SerialPortManager.h"
#include "../data/WeighScaleTest.h"

class WeighScaleManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit WeighScaleManager(QObject *parent = nullptr);

    void loadSettings(const QSettings &) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

public slots:

    // connect to the serial port
    // opens the serial port with required parametere (baud rate etc.)
    // connects the port readyRead signal to the readDevice slot
    // emits canMeasure signal if the port is open
    //
    void connectDevice() override;

    // zero the weigh scale
    //
    void zeroDevice();

    // retrieve a measurement from the device
    //
    void measure() override;

    // implementation of final clean up of device after disconnecting and all
    // data has been retrieved and processed by any upstream classes
    //
    void finish() override;

private slots:

    // retrieve data from the scale over RS232
    // emits canWrite signal if the test data is valid
    // Read is based on the last written code
    //
    void readDevice() override;

    void writeDevice() override;

private:

    WeighScaleTest m_test;

    void clearData() override;
};

#endif // WEIGHSCALEMANAGER_H
