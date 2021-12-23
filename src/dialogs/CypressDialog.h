#ifndef CYPRESSDIALOG_H
#define CYPRESSDIALOG_H

#include <QDialog>
#include <QObject>
#include "../CypressApplication.h"
#include <QStatusBar>
#include <QLineEdit>

class CypressDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CypressDialog(QWidget *parent = nullptr);
    ~CypressDialog();

    void initialize(const CypressApplication::TestType &type);
    void setStatusMessage(const QString &);
    QString getBarcode() const;

private:
    QSharedPointer<QStatusBar> m_status;
    QSharedPointer<QLineEdit> m_barcodeEdit;

};

#endif // CYPRESSDIALOG_H
