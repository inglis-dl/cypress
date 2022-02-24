#ifndef CYPRESSCONSTANTS_H
#define CYPRESSCONSTANTS_H

#include <QObject>

class CypressConstants
{
   Q_GADGET

public:

    enum RunMode {
        Unknown,
        Default,
        Live,
        Simulate
    };
    Q_ENUM(RunMode)

    enum MeasureType {
        None,
        Audiometer,
        BloodPressure,
        BodyCompositionAnalyzer,
        CDTT,
        ChoiceReaction,
        ECG,
        Frax,
        RetinalCamera,
        Spirometer,
        Thermometer,
        Tonometer,
        WeighScale
    };
    Q_ENUM(MeasureType)

    static MeasureType getMeasureType(const QString&);
    static RunMode getRunMode(const QString&);

    static QString getMeasureTypeName(const MeasureType&);
    static QString getRunModeName(const RunMode&);
};

#endif // CYPRESSCONSTANTS_H
