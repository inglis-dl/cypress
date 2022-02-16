#ifndef DIALOGFACTORY_H
#define DIALOGFACTORY_H

#include <QObject>
#include <QSharedPointer>
#include <QMap>

QT_FORWARD_DECLARE_CLASS(DialogBase)

class DialogFactory
{
public:
    static DialogFactory *instance();
    ~DialogFactory();

    enum Type {
        None,
        Spirometer,
        WeighScale,
        BodyCompositionAnalyzer,
        Frax,
        CDTT,
        BloodPressure,
        Thermometer,
        Audiometer,
        ChoiceReaction,
        Tonometer,
        RetinalCamera,
        ECG
    };

    DialogBase* instantiate(const Type&);
    DialogBase* instantiate(const QString&);

    typedef QMap<QString,DialogFactory::Type> lutType;
    static lutType initTypeLUT();

    static DialogFactory::Type getType(const QString&);

private:
    DialogFactory() = default;
    static DialogFactory *pInstance;
    static lutType typeLUT;
};

#endif // DIALOGFACTORY_H
