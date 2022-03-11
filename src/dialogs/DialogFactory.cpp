#include "DialogFactory.h"

#include "AudiometerDialog.h"
#include "BloodPressureDialog.h"
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
      case Constants::MeasureType::typeWeighScale:
        dialog = new WeighScaleDialog();
        break;
      case Constants::MeasureType::typeBodyComposition:
        dialog = new BodyCompositionDialog();
        break;
      case Constants::MeasureType::typeAudiometer:
        dialog = new AudiometerDialog();
        break;
      case Constants::MeasureType::typeChoiceReaction:
        dialog = new ChoiceReactionDialog();
        break;
      case Constants::MeasureType::typeThermometer:
        dialog = new ThermometerDialog();
        break;
      case Constants::MeasureType::typeFrax:
        dialog = new FraxDialog();
        break;
      case Constants::MeasureType::typeCDTT:
        dialog = new CDTTDialog();
        break;
      case Constants::MeasureType::typeTonometer:
        dialog = new TonometerDialog();
        break;
      case Constants::MeasureType::typeSpirometer:
        //dialog = Q_NULLPTR;
        break;
      case Constants::MeasureType::typeBloodPressure:
        dialog = new BloodPressureDialog();
        break;
      case Constants::MeasureType::typeRetinalCamera:
        //dialog = Q_NULLPTR;
        break;
      case Constants::MeasureType::typeECG:
        //dialog = Q_NULLPTR;
        break;
      case Constants::MeasureType::typeUnknown:
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
