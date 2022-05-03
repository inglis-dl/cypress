#ifndef GRIPSTRENGTHMANAGER_H
#define GRIPSTRENGTHMANAGER_H

#include "ManagerBase.h"
#include "../data/GripStrengthTest.h"

#include <QProcess>

class GripStrengthManager : public ManagerBase
{
    enum FileType {
        Tracker5Exe,
        GripTestDbPath,
        GripTestDataDbPath
    };
    Q_OBJECT
public:
    explicit GripStrengthManager(QObject* parent = Q_NULLPTR);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void initializeModel() override;

    void updateModel() override;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QVariantMap&) override;

    bool isDefined(const QString&, const GripStrengthManager::FileType&) const;

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

private:
    QString m_runnableName = "C:/Program Files (x86)/Tracker 5/WTS.exe";// full pathspec to WTS.exe
    QString m_runnablePath = "C:/Program Files (x86)/Tracker 5"; // path to WTS.exe directory
    QString m_gripTestDbPath = "C:/Users/clsa/Desktop/ZGripTest.DB";
    QString m_gripTestDataDbPath = "C:/Users/clsa/Desktop/ZGripTestData.DB";

    //QString getEMRInXmlName() const { return QString("%1/%2").arg(m_dataPath, "CypressIn.xml"); }
    //QString getEMROutXmlName() const { return QString("%1/%2").arg(m_dataPath, "CypressOut.xml"); }
    //QString getEWPDbName() const { return QString("%1/%2").arg(m_dataPath, "EasyWarePro.mdb"); }
    //QString getEWPDbCopyName() const { return QString("%1/%2").arg(m_dataPath, "EasyWareProCopy.mdb"); }
    //QString getEWPOptionsDbName() const { return QString("%1/%2").arg(m_dataPath, "EwpOptions.mdb"); }
    //QString getEWPOptionsDbCopyName() const { return QString("%1/%2").arg(m_dataPath, "EwpOptionsCopy.mdb"); }

    QProcess m_process;

    GripStrengthTest m_test;

    void clearData() override;

    void configureProcess();
};

#endif // GRIPSTRENGTHMANAGER_H
