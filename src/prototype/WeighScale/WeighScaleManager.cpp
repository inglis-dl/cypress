#include "WeighScaleManager.h"

WeighScaleManager::WeighScaleManager(QObject *parent) : QObject(parent),
    m_verbose(false)
{
}

void WeighScaleManager::loadSettings(const QSettings &settings)
{
}

void WeighScaleManager::saveSettings(QSettings *settings)
{
}

void WeighScaleManager::clearData()
{
    m_measurementData.clear();
    m_deviceData.clear();
    setProperty("weight", "");
    setProperty("datetime", "");
}
