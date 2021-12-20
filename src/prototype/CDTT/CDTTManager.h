#ifndef CDTTMANAGER_H
#define CDTTMANAGER_H

#include "../../managers/ManagerBase.h"
#include "../../data/CDTTTest.h"

#include <QProcess>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QSettings>

class CDTTManager : public ManagerBase
{
public:
    explicit CDTTManager(QObject* parent = nullptr);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel*) const override;

   // is the passed string a jar file
   // with the correct path elements ?
   //
    bool isDefined(const QString&) const;

    // set the cognitive test executable full path and name
    // calls isDefined to validate the passed arg
    //
    void setJarFullPath(const QString&);

    QString getJarFullPath() const
    {
        return m_jarFullPath;
    }

    // call just before closing the application to
    // remove the csv file from the test if it exists
    //
    void clean();

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QMap<QString, QVariant>&);

    QMap<QString, QVariant> m_inputData;

public slots:

    void measure() override;

    void readOutput();

private:
    QString m_jarFullPath; // full pathspec to CDTTstereo.jar
    QString m_jarDirPath; // path to CDTTstereo.jar directory
    QString m_outputPath;     // path to output .csv files
    QString m_outputFile;     // full pathspec to working output .csv file
    QProcess m_process;

    CDTTTest m_test;

    void clearData() override;

    QList<QString> m_inputKeyList;

    void configureProcess();
};

#endif // CDTTMANAGER_H
