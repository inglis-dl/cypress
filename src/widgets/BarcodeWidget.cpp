#include "BarcodeWidget.h"

#include <QRegExpValidator>
#include <QTimeLine>

BarcodeWidget::BarcodeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BarcodeWidget)
{
    ui->setupUi(this);

    m_lineEdit = this->findChild<QLineEdit*>("barcodeLineEdit");

    QRegExp rx("\\d{8}");
    QRegExpValidator *v_barcode = new QRegExpValidator(rx);
    m_lineEdit->setValidator(v_barcode);

    m_timeLine = new QTimeLine(2000,this);
    m_timeLine->setFrameRange(0,255);
    m_timeLine->setLoopCount(0);
    m_timeLine->setObjectName("timer");
    connect(m_timeLine, &QTimeLine::frameChanged,
            this,[this](int frame){
        auto p = m_lineEdit->palette();
        p.setBrush(QPalette::Base,QBrush(QColor(255,255,0,frame)));
        m_lineEdit->setPalette(p);
    });
    connect(m_timeLine, &QTimeLine::finished, m_timeLine, &QTimeLine::deleteLater);
    m_timeLine->start();

    connect(m_lineEdit, &QLineEdit::returnPressed,
            this,[this](){
        bool valid = false;
        if(this->isValid())
        {
            m_timeLine->stop();
            m_timeLine->setCurrentTime(0);
            auto p = m_lineEdit->palette();
            p.setBrush(QPalette::Base,QBrush(QColor(0,255,0,128)));
            m_lineEdit->setPalette(p);
            m_lineEdit->repaint();

            valid = true;
        }
        emit validated(valid);
    });
}

BarcodeWidget::~BarcodeWidget()
{
    m_lineEdit = Q_NULLPTR;
    delete m_timeLine;
    m_timeLine = Q_NULLPTR;
    delete ui;
}

// the target barcode to compare QLineEdit entries against
//
QString BarcodeWidget::barcode() const
{
    return m_barcode;
}

// the target barcode to compare QLineEdit entries against
//
void BarcodeWidget::setBarcode(const QString &barcode)
{
    if(barcode != m_barcode)
    {
      m_barcode = barcode;
    }
}

bool BarcodeWidget::isValid() const
{
    QString str = m_lineEdit->text().simplified();
    str.replace(" ","");
    return str == m_barcode;
}
