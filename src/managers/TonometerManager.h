#ifndef TONOMETERMANAGER_H
#define TONOMETERMANAGER_H

#include "ManagerBase.h"
#include "../data/TonometerTest.h"
#include <QProcess>

class TonometerManager : public ManagerBase
{
	Q_OBJECT

public:
    explicit TonometerManager(QObject* parent = nullptr);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

    // is the passed string an executable file
    // with the correct path elements ?
    //
    bool isDefined(const QString &) const;

    // set the executable full path and name
    // calls isDefined to validate the passed arg
    //
    void setRunnableName(const QString &);

    QString getRunnableName() const
    {
        return m_runnableName;
    }

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QMap<QString,QVariant> &);

public slots:

    // retrieve a measurement from the device
    //
    void measure() override;

    // implementation of final clean up of device after disconnecting and all
    // data has been retrieved and processed by any upstream classes
    //
    void finish() override;

    void readOutput();

    // ManagerBase impl requirement stub
    //
    void start() override {};

private:
    QString m_runnableName;// full pathspec to ora.exe
    QString m_runnablePath;// path to ora.exe

    QString m_outputFile;    // full pathspec to working output.txt
    QString m_inputFile;     // full pathspec to working input.txt
    QString m_temporaryFile; // store a copy of the default input.txt
    QProcess m_process;

    TonometerTest m_test;

    void clearData() override;

    QMap<QString,QVariant> m_inputData;
    QList<QString> m_inputKeyList;

    void configureProcess();
};

#endif // TONOMETERMANAGER_H
