#include "ManagerBase.h"

ManagerBase::ManagerBase(QObject *parent) : QObject(parent),
    m_verbose(false),
    m_mode("default")
{
}
