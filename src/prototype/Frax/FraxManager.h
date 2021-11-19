#ifndef FRAXMANAGER_H
#define FRAXMANAGER_H

#include "../../managers/ManagerBase.h"
//#include "../../data/FraxTest.h"

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

    void buildModel(QStandardItemModel*) const override {};

    // is the passed string an executable file
    // with the correct path elements ?
    //
    bool isDefined(const QString&) const;

    // set the cognitive test executable full path and name
    // calls isDefined to validate the passed arg
    //
    void setExecutableName(const QString&);

    QString getExecutableName() const
    {
        return m_executableName;
    }

    QString getExecutableFullPath() const
    {
        return m_executablePath;
    }

    QString getInputFullPath() const
    {
        return m_inputPath;
    }

    QString getOldInputFullPath() const
    {
        return m_oldInputPath;
    }

    QString getOutputFullPath() const
    {
        return m_outputPath;
    }

private:
    QString m_executableName;
    QString m_executablePath;
    QString m_outputPath;
    QString m_inputPath;
    QString m_oldInputPath;
    QProcess m_process;

    //FraxTest m_test;

    void clearData() override;

    QMap<QString, QVariant> m_inputData;
};

#endif // FRAXMANAGER_H