#ifndef BARCODEWIDGET_H
#define BARCODEWIDGET_H

#include <QWidget>
#include "ui_barcodewidget.h"

QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QTimeLine)

class BarcodeWidget : public QWidget, public Ui::BarcodeWidget
{
    Q_OBJECT

public:
    explicit BarcodeWidget(QWidget *parent = Q_NULLPTR);
    ~BarcodeWidget();

    QString barcode() const;

public slots:
    void setBarcode(const QString &);

signals:
    void validated(const bool&);

private:
    QLineEdit *m_lineEdit { Q_NULLPTR };
    QTimeLine *m_timeLine { Q_NULLPTR };
    QString m_barcode;
    Ui::BarcodeWidget *ui { Q_NULLPTR };

    bool isValid() const;
};

#endif // BARCODEWIDGET_H
