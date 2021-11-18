#ifndef TANITAMANAGER_H
#define TANITAMANAGER_H

#include "../../managers/SerialPortManager.h"
#include "../../data/BodyCompositionAnalyzerTest.h"

#include <QQueue>

class TanitaManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit TanitaManager(QObject *parent = nullptr);

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

    static QMap<QString,QByteArray> initCommandLookup();
    static QMap<QByteArray,QString> initResponseLookup();

signals:
   void canConfirm();

public slots:

    // connect to the serial port
    // opens the serial port with required parametere (baud rate etc.)
    // connects the port readyRead signal to the readDevice slot
    // emits canMeasure signal if the port is open
    //
    void connectDevice() override;

    // reset all device settings
    //
    void resetDevice();

    void confirmSettings();

    void setInputs(const QMap<QString,QVariant> &);

    void writeDevice() override;
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
    static QMap<QString,QByteArray> commandLookup;
    static QMap<QByteArray,QString> responseLookup;

    bool hasEndCode(const QByteArray &);

    BodyCompositionAnalyzerTest m_test;

    void clearData() override;

    QVector<QByteArray> m_cache;
    QQueue<QByteArray> m_queue;
};

#endif // TANITAMANAGER_H
