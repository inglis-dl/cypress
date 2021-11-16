#ifndef TANITAMANAGER_H
#define TANITAMANAGER_H

#include "../../managers/SerialPortManager.h"
#include "../../data/BodyCompositionAnalyzerTest.h"

class TanitaManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit TanitaManager(QObject *parent = nullptr);

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

private slots:

    // retrieve data from the scale over RS232
    // emits canWrite signal if the test data is valid
    // Read is based on the last written code
    //
    void readDevice() override;

private:

    bool hasEndCode(const QByteArray &);

    BodyCompositionAnalyzerTest m_test;

    void clearData() override;
};

#endif // TANITAMANAGER_H
