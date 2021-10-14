#ifndef HEARINGMEASUREMENT_H
#define HEARINGMEASUREMENT_H

#include "MeasurementBase.h"

/*!
 * \class HearingMeasurement
 * \brief A HearingMeasurement class
 *
 * Measurements of weight are derived from a
 * Rice Lake digital weigh scale over RS232 serial
 * communication.  This class facilitates parsing
 * QByteArray input from the QSerialPort of the WeighScaleManager
 * class into measurement characteristics, such as value, units etc.
 *
 * \sa MeasurementBase
 */

class HearingMeasurement :  public MeasurementBase
{   
public:
    void fromArray(const QByteArray &);

    QString toString() const;

    bool isValid() const;

    static bool hasEndCode(const QByteArray &);

    char getFlag() const;
    QString getPatientID() const;
    QString getTestID() const;
    QDateTime getTestDateTime() const;
    QDate getCalibrationDate() const;
    QString getExaminerID() const;
    QMap<QString,quint8> getLeftHearingTestLevels() const;
    QMap<QString,quint8> getRightHearingTestLevels() const;
    QMap<QString,QString> getLeftHearingTestCodes() const;
    QMap<QString,QString> getRightHearingTestCodes() const;
    QMap<QString,QString> getLeftHearingTestOutcomes() const;
    QMap<QString,QString> getRightHearingTestOutcomes() const;

private:
    QByteArray m_array;

    QString readArray(const quint8 &begin, const quint8 &end) const;
};

Q_DECLARE_METATYPE(HearingMeasurement);

#endif // HEARINGMEASUREMENT_H
