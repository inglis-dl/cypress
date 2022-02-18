#ifndef BLOODPRESSUREMANAGER_H
#define BLOODPRESSUREMANAGER_H

#include "ManagerBase.h"
#include "../data/BloodPressureTest.h"
#include "../prototype/BloodPressure/BPM200.h"

class BloodPressureManager : public ManagerBase
{
public:
    explicit BloodPressureManager(QObject* parent = Q_NULLPTR);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel*) const override;

    // Inherited via ManagerBase
    void connectUI(QWidget*) override;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QMap<QString, QVariant>&);

    void SetupConnections();
public slots:

    void start() override;

    // retrieve a measurement from the device
    //
    void measure() override;

    // implementation of final clean up of device after disconnecting and all
    // data has been retrieved and processed by any upstream classes
    //
    void finish() override;

    // slot for signals coming from bpm200
    void measurementAvailable(const int& sbp, const int& dbp, const int& pulse, const const QDateTime& start, const QDateTime& end, const int& readingNum);
    void averageAvailable(const int& sbp, const int& dbp, const int& pulse);
    void finalReviewAvailable(const int &sbp, const int &dbp, const int &pulse);
    void connectionStatusAvailable(const bool connected);
protected:
    void clearData() override;

private:
    BPM200 m_bpm;
    BloodPressureTest m_test;
};

#endif // BLOODPRESSUREMANAGER_H
