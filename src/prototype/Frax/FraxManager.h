#ifndef FRAXMANAGER_H
#define FRAXMANAGER_H

#include "../../managers/ManagerBase.h"
#include "../../data/FraxTest.h"

#include <QProcess>
#include <QDir>

class FraxManager : public ManagerBase
{
	Q_OBJECT
public:
    explicit FraxManager(QObject* parent = nullptr);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

    // is the passed string an executable file
    // with the correct path elements ?
    //
    bool isDefined(const QString &) const;

    // set the cognitive test executable full path and name
    // calls isDefined to validate the passed arg
    //
    void setExecutableName(const QString &);

    QString getExecutableName() const
    {
        return m_executableName;
    }

    // call just before closing the application to
    // remove the output txt file from the test if it exists
    //
    void clean();

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    void setInputData(const QMap<QString,QVariant> &);

public slots:

    void measure() override;

    void readOutput();

private:
    QString m_executableName;// full pathspec to blackbox.exe
    QString m_executablePath;// path to blackbox.exe
    QString m_outputFile;    // full pathspec to working output.txt
    QString m_inputFile;     // full pathspec to working input.txt
    QString m_temporaryFile; // store a copy of the default input.txt
    QProcess m_process;

    FraxTest m_test;

    void clearData() override;

    QMap<QString,QVariant> m_inputData;
    QList<QString> m_inputKeyList;

    void configureProcess();
};

#endif // FRAXMANAGER_H
