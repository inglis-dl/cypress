#ifndef BARCODEWIDGET_H
#define BARCODEWIDGET_H

#include <QWidget>
#include <QLineEdit>

class BarcodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BarcodeWidget(QWidget *parent = Q_NULLPTR);
    ~BarcodeWidget();

    QString getBarcode();

signals:
    void barcodeChanged(const QString &);

public slots:
    void setBarcode(const QString &);

private:
    QLineEdit *m_edit;

};

#endif // BARCODEWIDGET_H
