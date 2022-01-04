#ifndef AUDIOMETERMANAGER_H
#define AUDIOMETERMANAGER_H

#include "SerialPortManager.h"
#include "../data/HearingTest.h"

class AudiometerManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit AudiometerManager(QObject *parent = nullptr);

    void loadSettings(const QSettings &) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

public slots:

    // retrieve a measurement from the device
    //
    void measure() override;

    // implementation of final clean up of device after disconnecting and all
    // data has been retrieved and processed by any upstream classes
    //
    void finish() override;

private slots:

    // retrieve data from the audiometer over RS232
    // emits canWrite signal if the test data is valid
    //
    void readDevice() override;

    void writeDevice() override;

private:

    bool hasEndCode(const QByteArray &);

    HearingTest m_test;

    void clearData() override;
};

#endif // AUDIOMETERMANAGER_H
