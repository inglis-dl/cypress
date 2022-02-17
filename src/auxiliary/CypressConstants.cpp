#include "CypressConstants.h"

CypressConstants::lutType CypressConstants::initTypeLUT()
{
    CypressConstants::lutType lut;
    lut["weigh_scale"] = Type::WeighScale;
    lut["audiometer"] = Type::Audiometer;
    lut["spirometer"] = Type::Spirometer;
    lut["thermometer"] = Type::Thermometer;
    lut["frax"] = Type::Frax;
    lut["body_composition_analyzer"] = Type::BodyCompositionAnalyzer;
    lut["cdtt"] = Type::CDTT;
    lut["choice_reaction"] = Type::ChoiceReaction;
    lut["blood_pressure"] = Type::BloodPressure;
    lut["tonometer"] = Type::Tonometer;
    lut["retinal_camera"] = Type::RetinalCamera;
    lut["ecg"] = Type::ECG;
    return lut;
}

CypressConstants::lutType CypressConstants::typeLUT =
  CypressConstants::initTypeLUT();

CypressConstants::Type CypressConstants::getType(const QString& name)
{
  if(typeLUT.contains(name))
    return typeLUT[name];
  else
    return None;
}

CypressConstants::Mode CypressConstants::getMode(const QString& name)
{
  if("default" == name)
    return Mode::Default;
  else if("simulate"==name)
    return Mode::Simulate;
  else if("live"==name)
    return Mode::Live;
  else
    return Mode::Unknown;
}
