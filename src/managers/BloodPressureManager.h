#ifndef BLOODPRESSUREMANAGER_H
#define BLOODPRESSUREMANAGER_H

#include "ManagerBase.h"
#include "../data/BloodPressureTest.h"

#include <QObject>
#include <QThread>
#include <QUsb>

/*!
 * \class BloodPressureManager
 * \brief A Blood Presssure HID Device manager class
 *
 * Concrete child class implementation of a USB human interface device (HID)
 * manager.  This class facilitates connection to a blood pressure monitor
 * such as the BpTru BPM200.  Devices are identified and selected by product
 * and vendor ID pairs (ie., QUsb::Id).  Signals and slots are implemented
 * for UI selection from a list of discovered devices and for running a
 * blood pressure test.
 *
 * Communication with the device is facilitated by a worker class on a
 * separate thread with signal and slot connections between worker and manager.
 * The default vendor ID for BpTru can be overridden.
 *
 * \sa ManagerBase, BPMCommunication, BPMMessage, CRC8
 *
 */

QT_FORWARD_DECLARE_CLASS(BPMCommunication)

class BloodPressureManager : public ManagerBase
{
    Q_OBJECT

    Q_PROPERTY(QString deviceName MEMBER m_deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString cuffSize MEMBER m_cuffSize NOTIFY cuffSizeChanged)
    Q_PROPERTY(QString side MEMBER m_side NOTIFY sideChanged)

public:
    explicit BloodPressureManager(QObject *parent = Q_NULLPTR);
    ~BloodPressureManager();

    const quint16 BPTRU_VENDOR_ID { 4279 };

    bool isDefined(const QString&) const;

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel*) const override;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    // TODO: consider adding cuff size and arm side as json inputs
    //
    void setInputData(const QJsonObject&) override;

    //TODO: use cypress constant for all use of size and side
    void setCuffSize(const QString&);

    //TODO: use cypress constant for all use of size and side
    void setSide(const QString&);

    void setVendorIdFilter(const quint16& vid);

public slots:

    // what the manager does in response to the main application
    // window invoking its run method
    //
    void start() override;

    // retrieve a measurement from the device
    //
    void measure() override;

    // implementation of final clean up of device after disconnecting and all
    // data has been retrieved and processed by any upstream classes
    //
    void finish() override;

    void connectDevice();

    void disconnectDevice();

    // set the device by descritive label
    //
    void selectDevice(const QString&);

    void scanDevices();

private slots:

    // slot for signals coming from BPMCommunication
    //
    void measurementAvailable(const int&, const int&, const int&, const int&,
                              const QDateTime&, const QDateTime&);
    void averageAvailable(const int&, const int&, const int&);
    void finalReviewAvailable(const int&, const int&, const int&);
    void connectionStatusChanged(const bool&);
    void abortComplete(const bool&);
    void deviceInfoAvailable();

    // set the device
    //
    void setDevice(const QUsb::Id&);

signals:

    // signals to BPMCommunication
    //
    void attemptConnection(const QUsb::Id&);
    void startMeasurement();
    void abortMeasurement(QThread*);

    void canConnectDevice();
    void scanningDevices();
    void deviceDiscovered(const QString&);
    void canSelectDevice();
    void deviceSelected(const QString&);

    void deviceNameChanged(const QString&);
    void sideChanged(const QString&);
    void cuffSizeChanged(const QString&);

private:
    BloodPressureTest m_test;

    // communications handling
    QThread m_thread;
    BPMCommunication* m_comm { Q_NULLPTR };

    bool m_aborted { false };

    void clearData() override;

    // device data is separate from test data
    Measurement m_deviceData;

    // the usb hid device
    QString m_deviceName;
    QUsb::Id m_device;

    // usb hid devices plugged in and openable
    QMap<QString,QUsb::Id> m_deviceList;

    quint16 m_vendorIDFilter { 0 };

    QString m_cuffSize { "" };
    QString m_side { "" };

    // called when loading from settings
    void selectDeviceById(const QUsb::Id&);
};

#endif // BLOODPRESSUREMANAGER_H
