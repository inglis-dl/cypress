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

DialogBase* DialogFactory::instantiate(const CypressConstants::MeasureType &type)
{
    DialogBase* dialog = Q_NULLPTR;
    switch(type)
    {
      case CypressConstants::MeasureType::WeighScale:
        dialog = new WeighScaleDialog();
        break;
      case CypressConstants::MeasureType::BodyCompositionAnalyzer:
        dialog = new BodyCompositionDialog();
        break;
      case CypressConstants::MeasureType::Audiometer:
        dialog = new AudiometerDialog();
        break;
      case CypressConstants::MeasureType::ChoiceReaction:
        dialog = new ChoiceReactionDialog();
        break;
      case CypressConstants::MeasureType::Thermometer:
        dialog = new ThermometerDialog();
        break;
      case CypressConstants::MeasureType::Frax:
        dialog = new FraxDialog();
        break;
      case CypressConstants::MeasureType::CDTT:
        dialog = new CDTTDialog();
        break;
      case CypressConstants::MeasureType::Tonometer:
        dialog = new TonometerDialog();
        break;
      case CypressConstants::MeasureType::Spirometer:
        //dialog = Q_NULLPTR;
        break;
      case CypressConstants::MeasureType::BloodPressure:
        //dialog = Q_NULLPTR;
        break;
      case CypressConstants::MeasureType::RetinalCamera:
        //dialog = Q_NULLPTR;
        break;
      case CypressConstants::MeasureType::ECG:
        //dialog = Q_NULLPTR;
        break;
      case CypressConstants::MeasureType::None:
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
    return instantiate(CypressConstants::getMeasureType(name));
}
