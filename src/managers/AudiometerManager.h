#ifndef AUDIOMETERMANAGER_H
#define AUDIOMETERMANAGER_H

#include "SerialPortManager.h"
#include "../data/HearingTest.h"

/*!
 * \class AudiometerManager
 * @brief The AudiometerManager class
 *
 * Concrete child class implementation of a RS232 serial
 * device manager.  This class facilitates connection to a
 * Tremetrics RA300 or RA300 Plus audiometer for read instructions
 * and test data retrieval.
 *
 * Caveats:
 * RA300 or RA300 Plus must be configured as follows:
 * - pulsed stimulus mode [special 2]
 * - baud rate set to 9600 [special 7]
 * - RA500 interface [special 5]
 *
 * \sa ManagerBase, SerialPortManager, BodyCompositionManager
 *
 */

class AudiometerManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit AudiometerManager(QObject* parent = Q_NULLPTR);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void initializeModel() override;

    void updateModel() override;

    static QByteArray initEndCode();
    static QByteArray END_CODE;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QVariantMap&) override;

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

    bool hasEndCode(const QByteArray&);

    HearingTest m_test;

    void clearData() override;
};

#endif // AUDIOMETERMANAGER_H
