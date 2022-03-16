#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QObject>

class Constants
{
   Q_GADGET

public:

    enum RunMode {
        modeUnknown,
        modeDefault,
        modeLive,
        modeSimulate,
        modeHeadless
    };
    Q_ENUM(RunMode)

    enum MeasureType {
        typeUnknown,
        typeAudiometer,
        typeBloodPressure,
        typeBodyComposition,
        typeCDTT,
        typeChoiceReaction,
        typeECG,
        typeFrax,
        typeRetinalCamera,
        typeSpirometer,
        typeThermometer,
        typeTonometer,
        typeWeighScale
    };
    Q_ENUM(MeasureType)

    enum UnitsSystem {
        systemUnknown,
        systemMetric,
        systemImperial
    };
    Q_ENUM(UnitsSystem)

    // the default barcode to use when no input.json is provided
    //
    static const QString DefaultBarcode;

    // the default system of units is metric
    //
    static const UnitsSystem DefaultUnitsSystem;

    // the default number of elements (5) to display when a
    // comma delimeted string of numbers is split
    //
    static const quint16 DefaultSplitLength;

    static MeasureType getMeasureType(const QString&);
    static RunMode getRunMode(const QString&);    
    static UnitsSystem getUnitsSystem(const QString&);

    static QString getMeasureTypeName(const MeasureType&);
    static QString getRunModeName(const RunMode&);
    static QString getUnitsSystemName(const UnitsSystem&);
};

#endif // CONSTANTS_H
