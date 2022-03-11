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

    // set the emr transfer directory full path and name
    // check if the passed in dir is a directory,
    // but does not have any way of knowing if the directory is correct or not
    //
    void selectEmrTransferDir(const QString& emrTransferDir);

    void readOutput();

signals:

    // no runnable available or the selected runnable is invalid
    // signal can be connected to a ui slot to launch a File select dialog
    //
    void canSelectRunnable();

    // no valid EMR transfer directory available 
    // signal can be connected to a ui slot to launch a directory select dialog
    //
    void canSelectEmrTransferDir();

private:
    QString m_runnableFullPath;// full pathspec to EasyWarePro.exe
    QString m_runnableDir; // path to EasyWarePro.exe directory

    QString m_emrTransferDir; // Path to the emr transfer directory
    QString getTransferInFilePath() const { return QString("%1/%2").arg(m_emrTransferDir).arg("OnyxIn.xml"); }
    QString getTransferOutFilePath() const { return QString("%1/%2").arg(m_emrTransferDir).arg("OnyxOut.xml"); }
    QString getDbPath() const { return QString("%1/%2").arg(m_emrTransferDir).arg("EasyWarePro.mdb"); }
    QString getDbCopyPath() const { return QString("%1/%2").arg(m_emrTransferDir).arg("EasyWareProCopy.mdb"); }
    QString getDbOptionsPath() const { return QString("%1/%2").arg(m_emrTransferDir).arg("EwpOptions.mdb"); }
    QString getDbOptionsCopyPath() const { return QString("%1/%2").arg(m_emrTransferDir).arg("EwpOptionsCopy.mdb");}

    QProcess m_process;

    TrueFlowSpirometerTest m_test;

    OnyxInXml m_onyxInXml;

    void clearData() override;

    // Completes setup required before launching 
    // and then launches easy on PC
    void launchEasyOnPc();

    // Reset the files in the emr transfer folder 
    // so it is as if easy on pc has never been run before
    void resetEmrTransferFiles() const;

    // Create a copy of the two databases used in the emr transfer folder
    void createDatabaseCopies() const;

    bool inputDataInitialized() const;

    void configureProcess();
};

#endif // TRUEFLOWSPIROMETERMANAGER_H
