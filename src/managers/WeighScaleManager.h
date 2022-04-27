#ifndef WEIGHSCALEMANAGER_H
#define WEIGHSCALEMANAGER_H

#include "SerialPortManager.h"
#include "../data/WeighScaleTest.h"

/*!
 * \class WeighScaleManager
 * \brief The WeighScaleManager class
 *
 * Concrete child class implementation of a RS232 serial
 * device manager.  This class facilitates connection to a
 * Rice Lake digital weigh scale for read/write instructions
 * and test data retrieval.
 *
 * \sa ManagerBase, SerialPortManager, BodyCompositionManager
 *
 */

class WeighScaleManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit WeighScaleManager(QObject* parent = Q_NULLPTR);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void initializeModel() override;

    void updateModel() override;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QVariantMap&) override;

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
