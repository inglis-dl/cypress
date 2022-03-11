#include "Constants.h"
#include <QMetaEnum>

const QString Constants::DefaultBarcode
  = "00000000";

const Constants::UnitsSystem Constants::DefaultUnitsSystem
  = UnitsSystem::systemMetric;

const quint16 Constants::DefaultSplitLength
  = 5;

Constants::MeasureType Constants::getMeasureType(const QString& name)
{
  QMetaEnum meta = QMetaEnum::fromType<MeasureType>();
  int result = meta.keyToValue(QString("type%1").arg(name).toStdString().c_str());
  return -1 == result ? Constants::MeasureType::typeUnknown : static_cast<Constants::MeasureType>(result);
}

Constants::RunMode Constants::getRunMode(const QString& name)
{
  QMetaEnum meta = QMetaEnum::fromType<RunMode>();
  int result = meta.keyToValue(QString("mode%1").arg(name).toStdString().c_str());
  return -1 == result ? Constants::RunMode::modeUnknown : static_cast<Constants::RunMode>(result);
}

Constants::UnitsSystem Constants::getUnitsSystem(const QString& name)
{
  QMetaEnum meta = QMetaEnum::fromType<UnitsSystem>();
  int result = meta.keyToValue(QString("system%1").arg(name).toStdString().c_str());
  return -1 == result ? Constants::UnitsSystem::systemUnknown : static_cast<Constants::UnitsSystem>(result);
}

QString Constants::getMeasureTypeName(const Constants::MeasureType &type)
{
  QMetaEnum meta = QMetaEnum::fromType<MeasureType>();
  return QString(meta.valueToKey(type)).replace("type","");
}

QString Constants::getRunModeName(const Constants::RunMode &mode)
{
  QMetaEnum meta = QMetaEnum::fromType<RunMode>();
  return QString(meta.valueToKey(mode)).replace("mode","");
}

QString Constants::getUnitsSystemName(const Constants::UnitsSystem &system)
{
  QMetaEnum meta = QMetaEnum::fromType<UnitsSystem>();
  return QString(meta.valueToKey(system)).replace("system","");
}

