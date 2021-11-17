#ifndef COGNITIVETESTMANAGER_H
#define COGNITIVETESTMANAGER_H

#include "../../managers/ManagerBase.h"

class CognitiveTestManager: public ManagerBase
{
    Q_OBJECT

public:
    explicit CognitiveTestManager(QObject *parent = nullptr);

    /// <summary>
	/// Launch the cognitive test
	/// </summary>
    bool launch(const QString &, const QString &);
	
	/// <summary>
	/// Move the results file to the proper output path
	/// </summary>
	/// <param name="path"></param>
    bool moveResultsFile(const QString &);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override {};

private:
    QString m_executable;
    QString m_ccbFolderPath;
    QString m_resultsFolderName;

    void clearData() override;

};

#endif // COGNITIVETESTMANAGER_H
