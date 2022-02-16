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

DialogBase* DialogFactory::instantiate(const CypressConstants::Type &type/*, QWidget *parent = Q_NULLPTR*/)
{
    DialogBase* dialog = Q_NULLPTR;
    switch(type)
    {
      case CypressConstants::Type::WeighScale:
        dialog = new WeighScaleDialog();
        break;
      case CypressConstants::Type::BodyCompositionAnalyzer:
        dialog = new BodyCompositionDialog();
        break;
      case CypressConstants::Type::Audiometer:
        dialog = new AudiometerDialog();
        break;
      case CypressConstants::Type::ChoiceReaction:
        dialog = new ChoiceReactionDialog();
        break;
      case CypressConstants::Type::Thermometer:
        dialog = new ThermometerDialog();
        break;
      case CypressConstants::Type::Frax:
        dialog = new FraxDialog();
        break;
      case CypressConstants::Type::CDTT:
        dialog = new CDTTDialog();
        break;
      case CypressConstants::Type::Tonometer:
        dialog = new TonometerDialog();
        break;
      case CypressConstants::Type::Spirometer:
        //dialog = Q_NULLPTR;
        break;
      case CypressConstants::Type::BloodPressure:
        //dialog = Q_NULLPTR;
        break;
      case CypressConstants::Type::RetinalCamera:
        //dialog = Q_NULLPTR;
        break;
      case CypressConstants::Type::ECG:
        //dialog = Q_NULLPTR;
        break;
      case CypressConstants::Type::None:
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
    return instantiate(CypressConstants::getType(name));
}
