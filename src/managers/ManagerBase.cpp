#include "ManagerBase.h"
#include <QDebug>

ManagerBase::ManagerBase(QObject *parent) : QObject(parent),
    m_verbose(false),
    m_mode("default")
{
}

QVariant ManagerBase::getInputDataValue(const QString &key)
{
    return m_inputData.contains(key) ? m_inputData[key] : QVariant();
}
