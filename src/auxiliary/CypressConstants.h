#ifndef CYPRESSCONSTANTS_H
#define CYPRESSCONSTANTS_H

#include <QObject>
#include <QMap>

class CypressConstants
{
public:

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

    typedef QMap<QString,CypressConstants::Type> lutType;
    static lutType initTypeLUT();

    static CypressConstants::Type getType(const QString&);

private:
    CypressConstants() = default;
    static lutType typeLUT;

};

#endif // CYPRESSCONSTANTS_H
