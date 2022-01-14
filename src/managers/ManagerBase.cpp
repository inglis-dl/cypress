#include "ManagerBase.h"
#include <QDebug>

ManagerBase::ManagerBase(QObject *parent) : QObject(parent),
    m_verbose(false),
    m_mode("default"),
    m_validBarcode(false)
{
}

QVariant ManagerBase::getInputDataValue(const QString &key)
{
    return m_inputData.contains(key) ? m_inputData[key] : QVariant();
}

bool ManagerBase::verifyBarcode(const QString &barcode)
{
    m_validBarcode = false;
    if(m_inputData.contains("barcode"))
    {
        QString str = barcode.simplified();
        str.replace(" ","");
        m_validBarcode = str == m_inputData["barcode"].toString();
    }
    return m_validBarcode;
}
