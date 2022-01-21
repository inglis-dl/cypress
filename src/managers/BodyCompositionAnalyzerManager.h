#ifndef BODYCOMPOSITIONANALYZERMANAGER_H
#define BODYCOMPOSITIONANALYZERMANAGER_H

#include "SerialPortManager.h"
#include "../data/BodyCompositionTest.h"
#include <QQueue>

class BodyCompositionAnalyzerManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit BodyCompositionAnalyzerManager(QObject *parent = Q_NULLPTR);

    void loadSettings(const QSettings &) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

    static QMap<QString,QByteArray> initDefaultLUT();
    static QMap<QByteArray,QString> initCommandLUT();
    static QMap<QByteArray,QString> initIncorrectResponseLUT();
    static QMap<QByteArray,QString> initConfirmationLUT();

    void setInputData(const QMap<QString,QVariant> &) override;

    void connectUI(QWidget *) override;

    static int AGE_MIN;
    static int AGE_MAX;
    static int HEIGHT_MIN_METRIC;
    static int HEIGHT_MAX_METRIC;
    static double HEIGHT_MIN_IMPERIAL;
    static double HEIGHT_MAX_IMPERIAL;
    static QByteArray END_CODE;

signals:
   // ready to receive the input map
   //
   void canInput();

   void canConfirm();

   void error(const QString &);

public slots:

    // connect to the serial port
    // opens the serial port with required parametere (baud rate etc.)
    // connects the port readyRead signal to the readDevice slot
    // emits canMeasure signal if the port is open
    //
    void connectDevice() override;

    // reset all device settings (removed inputs)
    //
    void resetDevice();

    // confirm the input after setInputs
    //
    void confirmSettings();

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
    static QMap<QString,QByteArray> defaultLUT;
    static QMap<QByteArray,QString> commandLUT;
    static QMap<QByteArray,QString> incorrectLUT;
    static QMap<QByteArray,QString> confirmLUT;

    bool hasEndCode(const QByteArray &) const;

    void processResponse(const QByteArray &, QByteArray);

    BodyCompositionTest m_test;

    void clearData() override;
    void clearQueue();

    QVector<QByteArray> m_cache;
    QQueue<QByteArray> m_queue;

//    QSharedPointer<QWidget> p_widget;
};

#endif // BODYCOMPOSITIONANALYZERMANAGER_H
