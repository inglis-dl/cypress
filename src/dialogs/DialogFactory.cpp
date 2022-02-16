#include "DialogFactory.h"

#include "AudiometerDialog.h"
//#include "BloodPressureDialog.h"
#include "ThermometerDialog.h"
#include "BodyCompositionDialog.h"
#include "CDTTDialog.h"
#include "ChoiceReactionDialog.h"
#include "FraxDialog.h"
#include "TonometerDialog.h"
#include "WeighScaleDialog.h"
#include <QDebug>

DialogFactory *DialogFactory::pInstance = Q_NULLPTR;

DialogFactory *DialogFactory::instance()
{
    if(!pInstance)
        pInstance = new DialogFactory();
    return pInstance;
}

DialogFactory::~DialogFactory()
{
    if(pInstance)
        pInstance = Q_NULLPTR;
}

DialogFactory::lutType DialogFactory::initTypeLUT()
{
    DialogFactory::lutType lut;
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

DialogFactory::lutType DialogFactory::typeLUT =
  DialogFactory::initTypeLUT();

DialogFactory::Type DialogFactory::getType(const QString& name)
{
  if(typeLUT.contains(name))
    return typeLUT[name];
  else
    return None;
}

DialogBase* DialogFactory::instantiate(const DialogFactory::Type &type/*, QWidget *parent = Q_NULLPTR*/)
{
    DialogBase* dialog = Q_NULLPTR;
    switch(type)
    {
      case Type::WeighScale:
        dialog = new WeighScaleDialog();
        break;
      case Type::BodyCompositionAnalyzer:
        dialog = new BodyCompositionDialog();
        break;
      case Type::Audiometer:
        dialog = new AudiometerDialog();
        break;
      case Type::ChoiceReaction:
        dialog = new ChoiceReactionDialog();
        break;
      case Type::Thermometer:
        dialog = new ThermometerDialog();
        break;
      case Type::Frax:
        dialog = new FraxDialog();
        break;
      case Type::CDTT:
        dialog = new CDTTDialog();
        break;
      case Type::Tonometer:
        dialog = new TonometerDialog();
        break;
      case Type::Spirometer:
        //dialog = Q_NULLPTR;
        break;
      case Type::BloodPressure:
        //dialog = Q_NULLPTR;
        break;
      case Type::RetinalCamera:
        //dialog = Q_NULLPTR;
        break;
      case Type::ECG:
        //dialog = Q_NULLPTR;
        break;
      case Type::None:
        //dialog = Q_NULLPTR;
        break;
      default:
        //dialog = Q_NULLPTR;
        break;
    }
    return dialog;
}

DialogBase* DialogFactory::instantiate(const QString& name)
{
    qDebug() << "factory making a" << name;
    return instantiate(getType(name));
}
