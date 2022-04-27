#include "ManagerBase.h"
#include <QStandardItemModel>

ManagerBase::ManagerBase(QObject *parent) : QObject(parent)
{
    m_model = new QStandardItemModel;
}

ManagerBase::~ManagerBase()
{
    delete m_model;
}

QVariant ManagerBase::getInputDataValue(const QString &key)
{
    return m_inputData.contains(key) ? m_inputData[key] : QVariant();
}
