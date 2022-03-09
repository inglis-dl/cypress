#include "ManagerBase.h"

ManagerBase::ManagerBase(QObject *parent) : QObject(parent)
{
}

QVariant ManagerBase::getInputDataValue(const QString &key)
{
    return m_inputData.contains(key) ? m_inputData[key] : QVariant();
}
