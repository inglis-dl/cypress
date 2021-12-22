#ifndef CYPRESSDIALOG_H
#define CYPRESSDIALOG_H

#include <QDialog>
#include <QObject>
#include <QStatusBar>
#include "../widgets/BarcodeWidget.h"

class CypressDialog : public QDialog
{
    Q_OBJECT
public:
    CypressDialog();
    ~CypressDialog();

    QString getBarcode();
    void setStatus(const QString &);

private:
  BarcodeWidget *m_widget;
  QStatusBar *m_status;
};

#endif // CYPRESSDIALOG_H
