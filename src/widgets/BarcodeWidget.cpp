#include "BarcodeWidget.h"

#include <QGroupBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>

BarcodeWidget::BarcodeWidget(QWidget *parent) : QWidget(parent)
{
  QGroupBox *group = new QGroupBox(this);
  group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  group->setTitle("barcode");
  QHBoxLayout *layout = new QHBoxLayout(this);
  group->setLayout(layout);
  QLabel *label = new QLabel(this);
  label->setText("Participant ID:");
  layout->addWidget(label);

  m_edit = new QLineEdit(this);
  m_edit->setInputMask("0 0 0 0 0 0 0 0;_");
  m_edit->setMaxLength(15);
  m_edit->setCursorPosition(15);
  m_edit->setClearButtonEnabled(true);
  layout->addWidget(m_edit);

  label->setBuddy(m_edit);

  layout->insertStretch(-1);

  connect(m_edit, &QLineEdit::textChanged,
          this,[this](const QString &text)
  {
    emit barcodeChanged(text);
  });
}

BarcodeWidget::~BarcodeWidget()
{
    delete m_edit;
}

QString BarcodeWidget::getBarcode()
{
    return m_edit->text();
}

void BarcodeWidget::setBarcode(const QString &text)
{
    m_edit->setText(text);
}
