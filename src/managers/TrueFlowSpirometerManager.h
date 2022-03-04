#ifndef TRUEFLOWSPIROMETERMANAGER_H
#define TRUEFLOWSPIROMETERMANAGER_H

#include "../../src/managers/ManagerBase.h"
#include "../data/TrueFlowSpirometerTest.h"
#include "../prototype/TrueFlowSpirometer/OnyxInXml.h"

#include <QProcess>

class TrueFlowSpirometerManager : public ManagerBase
{
    Q_OBJECT

public:
    explicit TrueFlowSpirometerManager(QObject* parent = Q_NULLPTR);
    ~TrueFlowSpirometerManager();

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel*) const override;

    // is the passed string a jar file
    // with the correct path elements ?
    //
    bool isDefined(const QString&) const;

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

    // set the executable full path and name
    // calls isDefined to validate the passed arg
    //
    void selectRunnable(const QString&);

    void readOutput();

signals:

    // a valid runnable was selected
    // manager attempts to configure the process and may emit canMeasure on success
    //
    void runnableSelected();

    // no runnable available or the selected runnable is invalid
    // signal can be connected to a ui slot to launch a File select dialog
    //
    void canSelectRunnable();

private:
    QString m_runnableFullPath;// full pathspec to EasyWarePro.exe
    QString m_runnableDir; // path to EasyWarePro.exe directory

    QString m_transferInFilePath;     // path to output .csv files
    QString m_transferOutFilePath;     // path to output .csv files
    QProcess m_process;

    TrueFlowSpirometerTest m_test;

    OnyxInXml m_onyxInXml;

    void clearData() override;

    void LaunchEasyOnPc();
    void ResetEasyOnPcFiles(const QString& dirPath) const;

    void configureProcess();
};

#endif // TRUEFLOWSPIROMETERMANAGER_H
