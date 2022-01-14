#ifndef TEMPLATEMANAGER_H
#define TEMPLATEMANAGER_H

#include "../../src/managers/ManagerBase.h"
#include "../data/TemplateTest.h"

class TemplateManager : public ManagerBase
{
public:
    explicit TemplateManager(QObject* parent = nullptr);

    void loadSettings(const QSettings&) override;
    void saveSettings(QSettings*) const override;

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel*) const override;

        // Set the input data.
        // The input data is read from the input
        // json file to the main application.  This method should be
        // used to filter the minimum inputs needed to run
        // a test.  Filtering keys are stored in member
        // m_inputKeyList.
        //
        void setInputData(const QMap<QString, QVariant>&);

public slots:

    // retrieve a measurement from the device
    //
    void measure() override;

    // implementation of final clean up of device after disconnecting and all
    // data has been retrieved and processed by any upstream classes
    //
    void finish() override;

    void readOutput();

    void start() override;

private:
    TemplateTest m_test;

    void clearData() override;

    QMap<QString, QVariant> m_inputData;
    QList<QString> m_inputKeyList;
};

#endif // TEMPLATEMANAGER_H