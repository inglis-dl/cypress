#ifndef SPIROMETERMANAGER_H
#define SPIROMETERMANAGER_H

#include "ManagerBase.h"
#include "../data/SpirometerTest.h"

#include <QProcess>

class SpirometerManager : public ManagerBase
{
    enum FileType {
        EasyWareExe,
        EMRDataPath
    };
    Q_OBJECT

public:
    explicit SpirometerManager(QObject* parent = Q_NULLPTR);
    ~SpirometerManager() = default;

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel*) const override;

    bool isDefined(const QString&, const SpirometerManager::FileType&) const;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QVariantMap&) override;

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

    void select();

    // set the executable full path and name
    // calls isDefined to validate the passed arg
    //
    void selectRunnable(const QString&);

    // set the emr transfer directory full path and name
    // check if the passed in dir is a directory,
    // but does not have any way of knowing if the directory is correct or not
    //
    void selectDataPath(const QString&);

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

    // no valid EMR data transfer directory available
    // signal can be connected to a ui slot to launch a directory select dialog
    //
    void canSelectDataPath();

    // a valid data path was selected
    // manager attempts to configure the process and may emit canMeasure on success
    //
    void dataPathSelected();

private:
    QString m_runnableName;// full pathspec to EasyWarePro.exe
    QString m_runnablePath; // path to EasyWarePro.exe directory
    QString m_dataPath; // Path to the EMR plugin data transfer directory

    QString getEMRInXmlName() const { return QString("%1/%2").arg(m_dataPath,"CypressIn.xml"); }
    QString getEMROutXmlName() const { return QString("%1/%2").arg(m_dataPath,"CypressOut.xml"); }
    QString getEWPDbName() const { return QString("%1/%2").arg(m_dataPath,"EasyWarePro.mdb"); }
    QString getEWPDbCopyName() const { return QString("%1/%2").arg(m_dataPath,"EasyWareProCopy.mdb"); }
    QString getEWPOptionsDbName() const { return QString("%1/%2").arg(m_dataPath,"EwpOptions.mdb"); }
    QString getEWPOptionsDbCopyName() const { return QString("%1/%2").arg(m_dataPath,"EwpOptionsCopy.mdb");}

    QProcess m_process;

    SpirometerTest m_test;

    void clearData() override;

    // create a copy of the two databases in the EMR transfer directory
    //
    void backupDatabases() const;

    // restore databases from copies
    //
    void restoreDatabases() const;

    void removeXmlFiles() const;

    void configureProcess();

    QString getOutputPdfPath() const;
    bool outputPdfExists() const;
};

#endif // SPIROMETERMANAGER_H
