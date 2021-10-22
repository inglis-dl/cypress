#ifndef AUDIOMETERTEST_H
#define AUDIOMETERTEST_H

#include "TestBase.h"
#include "HearingMeasurement.h"

class AudiometerTest : TestBase
{
public:
    AudiometerTest() = default;
    ~AudiometerTest() = default;
    AudiometerTest(const AudiometerTest &);
    AudiometerTest &operator=(const AudiometerTest &);

    void fromArray(const QByteArray &);

    QString toString() const override;

    bool isValid() const override;

    void reset() override;

    static bool hasEndCode(const QByteArray &);

    QString   getPatientID() const;
    QString   getTestID() const;
    QDateTime getTestDateTime() const;
    QDate     getCalibrationDate() const;
    QString   getExaminerID() const;
    QList<HearingMeasurement> getHearingThresholdLevels(const QString& side) const;

private:
    QByteArray m_array;
    QString readArray(const quint8 &, const quint8 &) const;
};

Q_DECLARE_METATYPE(AudiometerTest);

QDebug operator<<(QDebug dbg, const AudiometerTest &);

#endif // AUDIOMETERTEST_H
