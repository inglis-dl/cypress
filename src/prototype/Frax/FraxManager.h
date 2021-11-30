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

    void buildModel(QStandardItemModel*) const override;

    // is the passed string an executable file
    // with the correct path elements ?
    //
    bool isDefined(const QString&) const;

    // set the cognitive test executable full path and name
    // calls isDefined to validate the passed arg
    //
    void setExecutableName(const QString&);

    // call just before closing the application to
    // remove the output txt file from the test if it exists
    //
    void clean();

    QString getExecutableName() const
    {
        return m_executableFolderPath;
    }

    QString getExecutableFullPath() const
    {
        return m_executableFolderPath;
    }

    QString getInputFullPath() const
    {
        return m_inputFilePath;
    }

    QString getOldInputFullPath() const
    {
        return m_oldInputFilePath;
    }

    QString getOutputFullPath() const
    {
        return m_outputFilePath;
    }

    QMap<QString, QVariant> m_inputData;
    QMap<QString, QVariant> m_outputData;
public slots:
    void measure();

    void setInputs(const QMap<QString, QVariant>&);

    void readOutput();

signals:
    void canMeasure();

    void canWrite();
private:
    QString m_executableExePath;
    QString m_executableFolderPath;
    QString m_outputFilePath;
    QString m_inputFilePath;
    QString m_oldInputFilePath;
    QProcess m_process;

    FraxTest m_test;

    void configureProcess();

    void clearData() override;
    bool createInputsTxt();
    void readOutputs();
};

#endif // FRAXMANAGER_H