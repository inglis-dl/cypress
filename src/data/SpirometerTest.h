#ifndef SPIROMETERTEST_H
#define SPIROMETERTEST_H

#include "TestBase.h"
#include "SpirometerMeasurement.h"
#include <QJsonObject>

QT_FORWARD_DECLARE_CLASS(QDomNode)

class SpirometerTest : public TestBase<SpirometerMeasurement>
{

public:
    SpirometerTest();
    ~SpirometerTest() = default;

    void fromFile(const QString&);

    void simulate(const QVariantMap&);

    // String representation for debug and GUI display purposes
    //
    QString toString() const override;

    QList<QStringList> toStringListList() const;

    bool isValid() const override;

    // String keys are converted to snake_case
    //
    QJsonObject toJsonObject() const override;

    static const q_stringMap testMetaMap;
    static const q_stringMap patientMetaMap;

private:
    QStringList m_outputKeyList;

    void readPDFReportPath(const QDomNode&);
    void readPatients(const QDomNode&);
    void readTrials(const QDomNode&);
    void readParameters(const QDomNode&, SpirometerMeasurement*);
    void readChannel(const QDomNode&, SpirometerMeasurement*);
    void readBestValues(const QDomNode&);
};

Q_DECLARE_METATYPE(SpirometerTest);

#endif // SPIROMETERTEST_H

