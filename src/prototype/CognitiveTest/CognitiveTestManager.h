#ifndef COGNITIVETESTMANAGER_H
#define COGNITIVETESTMANAGER_H

#include "../../managers/ManagerBase.h"
#include "../../data/ChoiceReactionTest.h"
#include <QProcess>

class CognitiveTestManager: public ManagerBase
{
    Q_OBJECT

public:
    explicit CognitiveTestManager(QObject *parent = nullptr);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override {};

public slots:
    void measure();

    void setInputs(const QMap<QString,QVariant> &);

    void readOutput();

signals:
    void canMeasure();

    void canWrite();

private:
    QString m_executableName;
    QString m_executablePath;
    QString m_outputPath;
    QProcess m_process;

    ChoiceReactionTest m_test;

    void clearData() override;

    QMap<QString,QVariant> m_inputData;

    void configureProcess();
};

#endif // COGNITIVETESTMANAGER_H
