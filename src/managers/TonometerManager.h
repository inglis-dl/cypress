#ifndef TONOMETERMANAGER_H
#define TONOMETERMANAGER_H

#include "ManagerBase.h"
#include "../data/TonometerTest.h"
#include <QProcess>

class TonometerManager : public ManagerBase
{
    enum FileType {
        ORAApplication,
        ORADatabase
    };

	Q_OBJECT

public:
    explicit TonometerManager(QObject* parent = Q_NULLPTR);
    ~TonometerManager();

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

    // is the passed string an executable file
    // with the correct path elements ?
    //
    bool isDefined(const QString &, TonometerManager::FileType type = ORAApplication) const;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QMap<QString,QVariant> &) override;

    void connectUI(QWidget *) override {};

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
    void selectRunnable(const QString &);

    void selectDatabase(const QString &);

    void select();

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

    void databaseSelected();

    void canSelectDatabase();

private:
    QString m_runnableName;// full pathspec to ora.exe
    QString m_runnablePath;// path to ora.exe
    QString m_databaseName;// full pathspec to ora.mdb

    QString m_temporaryFile; // store a copy of ora.mdb
    QProcess m_process;

    TonometerTest m_test;

    void clearData() override;

    void configureProcess();
};

#endif // TONOMETERMANAGER_H
