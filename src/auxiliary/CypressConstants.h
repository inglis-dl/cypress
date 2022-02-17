#ifndef CYPRESSCONSTANTS_H
#define CYPRESSCONSTANTS_H

#include <QObject>
#include <QMap>

class CypressConstants
{
public:

    enum Mode {
        Unknown,
        Live,
        Simulate,
        Default
    };

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
    static CypressConstants::Mode getMode(const QString&);

private:
    CypressConstants() = default;
    static lutType typeLUT;

};

Q_DECLARE_METATYPE(CypressConstants::Mode)
Q_DECLARE_METATYPE(CypressConstants::Type)

#endif // CYPRESSCONSTANTS_H
