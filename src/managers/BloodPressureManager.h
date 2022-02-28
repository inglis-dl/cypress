#ifndef BLOODPRESSUREMANAGER_H
#define BLOODPRESSUREMANAGER_H

#include "ManagerBase.h"
#include "../data/BloodPressureTest.h"

#include <QObject>
#include <QThread>

QT_FORWARD_DECLARE_CLASS(BPMCommunication)

class BloodPressureManager : public ManagerBase
{
    Q_OBJECT

public:
    explicit BloodPressureManager(QObject *parent = Q_NULLPTR);
    ~BloodPressureManager();

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
    void setInputData(const QMap<QString,QVariant>&) override;

    //TODO: use cypress constant for all use of size and side
    void setCuffSize(const QString&);

    //TODO: use cypress constant for all use of size and side
    void setSide(const QString&);

    int getPid() const { return m_pid; }
    int getVid() const { return m_vid; }

    bool connectionInfoSet() const;

    QList<int> findAllPids() const;

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

    // set the device by PID
    //
    void selectDevice(const QString&);

private slots:

    // slot for signals coming from bpm200
    void measurementAvailable(const int&, const int&, const int&,
                              const QDateTime&, const QDateTime&,
                              const int&);
    void averageAvailable(const int&, const int&, const int&);
    void finalReviewAvailable(const int&, const int&, const int&);
    void connectionStatusChanged(const bool&);
    void abortComplete(const bool&);
    void deviceInfoAvailable();

signals:

    void canConnectDevice();

    // Signals to comm
    void attemptConnection(const int&, const int&);
    void startMeasurement();
    void abortMeasurement(QThread*);

private:

    bool armInformationSet() const { return m_test.armInformationSet(); }
    BloodPressureTest m_test;
    QThread m_thread;

    const int m_vid = 4279;  // vendor ID for BpTru
    int m_pid { 0 };

    BPMCommunication* m_comm;

    bool m_aborted { false };
    bool m_connectionsSet { false };

    void clearData() override;

    // device data is separate from test data
    MeasurementBase m_deviceData;
};

#endif // BLOODPRESSUREMANAGER_H
