#ifndef ECGMANAGER_H
#define ECGMANAGER_H

#include "ManagerBase.h"
#include "../data/ECGTest.h"
#include <QProcess>

/*!
 * \class ECGManager
 * \brief The ECGManager class
 *
 * Concrete child class implementation of a device manager.
 * This class facilitates launch of the ECG (fracture risk assessment)
 * module (blackbox.exe) and reading the test output
 * .txt files it produces.  QProcess is used to facilitate operations.
 *
 * \sa ManagerBase, CDTTManager, ChoiceReactionManager
 *
 */

class ECGManager : public ManagerBase
{
	Q_OBJECT

public:
    explicit ECGManager(QObject* parent = Q_NULLPTR);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

    // is the passed string an executable file
    // with the correct path elements ?
    //
    bool isDefined(const QString &) const;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QJsonObject &) override;

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
    QString m_runnableName;// full pathspec to blackbox.exe
    QString m_runnablePath;// path to blackbox.exe

    QString m_outputFile;    // full pathspec to working output.txt
    QString m_inputFile;     // full pathspec to working input.txt
    QString m_temporaryFile; // store a copy of the default input.txt
    QProcess m_process;

    ECGTest m_test;

    void clearData() override;

    void configureProcess();
};

#endif // ECGMANAGER_H
