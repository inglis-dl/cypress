#ifndef CYPRESSDIALOG_H
#define CYPRESSDIALOG_H

#include <QDialog>
#include <QObject>
#include "../CypressApplication.h"
#include <QStatusBar>
#include <QLineEdit>
#include <QTableView>

QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

class CypressDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CypressDialog(QWidget *parent = nullptr);
    ~CypressDialog();

    void initialize(CypressApplication*);
    void setStatusMessage(const QString &);
    QString getBarcode() const;

    void updateTableView(QStandardItemModel*);

private:
    QSharedPointer<QStatusBar> m_status;
    QSharedPointer<QLineEdit> m_barcodeEdit;
    QSharedPointer<QTableView> m_tableView;
};

#endif // CYPRESSDIALOG_H
