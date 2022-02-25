#ifndef BLOODPRESSUREMANAGER_H
#define BLOODPRESSUREMANAGER_H

#include "ManagerBase.h"
#include "../data/BloodPressureTest.h"
#include "BPM200.h"

class BloodPressureManager : public ManagerBase
{
    Q_OBJECT

public:
    explicit BloodPressureManager(QObject* parent = Q_NULLPTR);

    // TODO: make this private. This is public so that it can send a signal 
    // to mainwindow since bloodPressureManager cannot have its own signals
    BPM200 m_bpm;

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

    void setArmBandSize(const QString&);
    void setArm(const QString&);

    void connectToBpm() { m_bpm.connectToBpm(); }
    int  getPid() const { return m_bpm.getPid(); }
    int  getVid() const { return m_bpm.getVid(); }
    bool connectionInfoSet() const;

signals:
    void canConnectDevice();

public slots:

    void start() override;

    // retrieve a measurement from the device
    //
    void measure() override;

    // implementation of final clean up of device after disconnecting and all
    // data has been retrieved and processed by any upstream classes
    //
    void finish() override;

    // set the device by PID
    //
    void setDevice(const int&);

    // slot for signals coming from bpm200
    void measurementAvailable(const int&, const int&, const int&,
                              const QDateTime&, const QDateTime&,
                              const int&);
    void averageAvailable(const int&, const int&, const int&);
    void finalReviewAvailable(const int&, const int&, const int&);
    void connectionStatusAvailable(const bool&);

private:

    void setupConnections();
    bool armInformationSet() const { return m_test.armInformationSet(); }
    BloodPressureTest m_test;

    void clearData() override;

};

#endif // BLOODPRESSUREMANAGER_H
