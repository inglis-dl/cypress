#ifndef CDTTMANAGER_H
#define CDTTMANAGER_H

#include "ManagerBase.h"
#include "../data/CDTTTest.h"
#include <QProcess>

/*!
 * \class CDTTManager
 * \brief The CDTTManager class
 *
 * Concrete child class implementation of a device manager.
 * This class facilitates launch of the Canadian Digit Triplet Test
 * Java .jar program (CDTTstereo.jar) and reading the test output
 * .xml files it produces.  QProcess is used to facilitate exe operations
 * along with helper class ExcelQueryHelper to read the MS Excel file.
 *
 * \sa ManagerBase, ExcelQueryHelper, ChoiceReactionManager, FraxManager
 *
 */

class CDTTManager : public ManagerBase
{
    Q_OBJECT

public:
    explicit CDTTManager(QObject* parent = Q_NULLPTR);
    ~CDTTManager();

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void initializeModel() override;

    void updateModel() override;

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
    QString m_runnableName;// full pathspec to CDTTstereo.jar
    QString m_runnablePath;// path to CDTTstereo.jar

    QString m_outputPath;     // path to output .csv files
    QString m_outputFile;     // full pathspec to working output .csv file
    QProcess m_process;

    CDTTTest m_test;

    void clearData() override;

    void configureProcess();
};

#endif // CDTTMANAGER_H
