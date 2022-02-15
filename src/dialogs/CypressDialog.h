#ifndef CYPRESSDIALOG_H
#define CYPRESSDIALOG_H

#include "../managers/ManagerBase.h"
#include <QDialog>
#include <QStandardItemModel>

class CypressDialog : public QDialog
{
    Q_OBJECT

public:
    CypressDialog(QWidget *parent = Q_NULLPTR);
    ~CypressDialog() = default;

    // This method internally calls readInput
    //
    void initialize();

    // Call after initialize, launch the application and run
    // the device
    //
    void run();

    void setInputFileName(const QString& name) { m_inputFileName = name; }
    QString inputFileName() { return m_inputFileName; }

    void setOutputFileName(const QString& name) { m_outputFileName = name; }
    QString outputFileName() { return m_outputFileName; }

    void setMode(const QString& mode) { m_mode = mode.toLower(); }
    QString mode() { return m_mode; }

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose(){ return m_verbose; }

    virtual QString getVerificationBarcode() const = 0;

public slots:
    void writeOutput();

protected:
    void closeEvent(QCloseEvent *event) override;

    ManagerBase *m_manager;
    QStandardItemModel m_model;
    QString m_inputFileName;
    QString m_outputFileName;
    QString m_mode;
    bool m_verbose;

    QMap<QString,QVariant> m_inputData;
    QMap<QString,QVariant> m_outputData;
    void readInput();

private:

    virtual void initializeModel() = 0;
    virtual void initializeConnections() = 0;
};

#endif // CYPRESSDIALOG_H
