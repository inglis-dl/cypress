#include "CypressConstants.h"
#include <QMetaEnum>

CypressConstants::MeasureType CypressConstants::getMeasureType(const QString& name)
{
  QMetaEnum meta = QMetaEnum::fromType<MeasureType>();
  int result = meta.keyToValue(name.toStdString().c_str());
  return -1 == result ? CypressConstants::MeasureType::None : static_cast<CypressConstants::MeasureType>(result);
}

CypressConstants::RunMode CypressConstants::getRunMode(const QString& name)
{
  QMetaEnum meta = QMetaEnum::fromType<RunMode>();
  int result = meta.keyToValue(name.toStdString().c_str());
  return -1 == result ? CypressConstants::RunMode::Unknown : static_cast<CypressConstants::RunMode>(result);
}

QString CypressConstants::getMeasureTypeName(const CypressConstants::MeasureType &type)
{
  QMetaEnum meta = QMetaEnum::fromType<MeasureType>();
  return QString(meta.valueToKey(type));
}

QString CypressConstants::getRunModeName(const CypressConstants::RunMode &mode)
{
  QMetaEnum meta = QMetaEnum::fromType<RunMode>();
  return QString(meta.valueToKey(mode));
}
