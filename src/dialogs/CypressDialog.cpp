#include "CypressDialog.h"
#include <QVBoxLayout>

CypressDialog::CypressDialog()
{
  QDialog::setGeometry(0,0,561,541);

  QVBoxLayout *layout = new QVBoxLayout(this);
  m_status = new QStatusBar(this);

  m_widget = new BarcodeWidget(this);
  layout->addWidget(m_widget);
  layout->addWidget(m_status);
}

CypressDialog::~CypressDialog()
{
    delete m_widget;
    delete m_status;
}

QString CypressDialog::getBarcode()
{
  return m_widget->getBarcode();
}

void CypressDialog::setStatus(const QString &message)
{
  m_status->showMessage(message);
}
