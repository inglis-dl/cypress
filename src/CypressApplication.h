#ifndef CYPRESSAPPLICATION_H
#define CYPRESSAPPLICATION_H

#include "./auxiliary/Constants.h"
#include <QObject>
#include <QCoreApplication>

QT_FORWARD_DECLARE_CLASS(DialogBase)

class CypressApplication : public QObject
{
    Q_OBJECT
public:
    explicit CypressApplication(QObject *parent = Q_NULLPTR);
    ~CypressApplication();

    void setArgs(const QMap<QString,QVariant>&);
    void initialize();

private:
    QString m_inputFileName;
    QString m_outputFileName;
    Constants::RunMode m_mode { Constants::RunMode::modeUnknown };
    Constants::MeasureType m_type { Constants::MeasureType::typeUnknown };
    bool m_verbose;

    QScopedPointer<DialogBase> m_dialog;
};

#endif // CYPRESSAPPLICATION_H
