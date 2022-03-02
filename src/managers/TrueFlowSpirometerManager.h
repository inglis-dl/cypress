#ifndef TRUEFLOWSPIROMETERMANAGER_H
#define TRUEFLOWSPIROMETERMANAGER_H

#include "../../src/managers/ManagerBase.h"
#include "../data/TrueFlowSpirometerTest.h"

class TrueFlowSpirometerManager : public ManagerBase
{
public:
    explicit TrueFlowSpirometerManager(QObject* parent = Q_NULLPTR);

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
    void setInputData(const QMap<QString, QVariant> &) override;

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

    void readOutput();

private:
    TrueFlowSpirometerTest m_test;

    void clearData() override;
};

#endif // TRUEFLOWSPIROMETERMANAGER_H
