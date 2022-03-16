#ifndef AUDIOMETERMANAGER_H
#define AUDIOMETERMANAGER_H

#include "SerialPortManager.h"
#include "../data/HearingTest.h"

class AudiometerManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit AudiometerManager(QObject *parent = Q_NULLPTR);

    void loadSettings(const QSettings &) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

    static QByteArray initEndCode();
    static QByteArray END_CODE;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QJsonObject &) override;

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
