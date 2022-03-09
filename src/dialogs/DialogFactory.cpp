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

DialogBase* DialogFactory::instantiate(const Constants::MeasureType &type)
{
    DialogBase* dialog = Q_NULLPTR;
    switch(type)
    {
      case Constants::MeasureType::WeighScale:
        dialog = new WeighScaleDialog();
        break;
      case Constants::MeasureType::BodyCompositionAnalyzer:
        dialog = new BodyCompositionDialog();
        break;
      case Constants::MeasureType::Audiometer:
        dialog = new AudiometerDialog();
        break;
      case Constants::MeasureType::ChoiceReaction:
        dialog = new ChoiceReactionDialog();
        break;
      case Constants::MeasureType::Thermometer:
        dialog = new ThermometerDialog();
        break;
      case Constants::MeasureType::Frax:
        dialog = new FraxDialog();
        break;
      case Constants::MeasureType::CDTT:
        dialog = new CDTTDialog();
        break;
      case Constants::MeasureType::Tonometer:
        dialog = new TonometerDialog();
        break;
      case Constants::MeasureType::Spirometer:
        //dialog = Q_NULLPTR;
        break;
      case Constants::MeasureType::BloodPressure:
        //dialog = Q_NULLPTR;
        break;
      case Constants::MeasureType::RetinalCamera:
        //dialog = Q_NULLPTR;
        break;
      case Constants::MeasureType::ECG:
        //dialog = Q_NULLPTR;
        break;
      case Constants::MeasureType::None:
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
    return instantiate(Constants::getMeasureType(name));
}
