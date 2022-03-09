#include "HearingMeasurement.h"

#include <QDateTime>
#include <QDebug>
#include <QRegExp>

const QMap<QString,QString> HearingMeasurement::codeLookup = HearingMeasurement::initCodeLookup();
const QMap<QString,QString> HearingMeasurement::outcomeLookup = HearingMeasurement::initOutcomeLookup();
const QMap<int,QString> HearingMeasurement::frequencyLookup = HearingMeasurement::initFrequencyLookup();

 /*
  *    opal expects
  *
RES_TEST_ID text
RES_TEST_DATETIM datetime
RES_LAST_CALIBRATION_DATE datetime
RES_RIGHT_1KT integer
RES_RIGHT_500 integer
RES_RIGHT_1K integer
RES_RIGHT_2K integer
RES_RIGHT_3K integer
RES_RIGHT_4K integer
RES_RIGHT_6K integer
RES_RIGHT_8K integer
RES_LEFT_1KT integer
RES_LEFT_500 integer
RES_LEFT_1K integer
RES_LEFT_2K integer
RES_LEFT_3K integer
RES_LEFT_4K integer
RES_LEFT_6K integer
RES_LEFT_8K integer
RES_RIGHT_1KT_ERR tex
RES_RIGHT_500_ERR text
RES_RIGHT_1K_ERR text
RES_RIGHT_2K_ERR text
RES_RIGHT_3K_ERR text
RES_RIGHT_4K_ERR text
RES_RIGHT_6K_ERR text
RES_RIGHT_8K_ERR text
RES_LEFT_1KT_ERR text
RES_LEFT_500_ERR text
RES_LEFT_1K_ERR text
RES_LEFT_2K_ERR text
RES_LEFT_3K_ERR text
RES_LEFT_4K_ERR text
RES_LEFT_6K_ERR text
RES_LEFT_8K_ERR
  *
  *
  */

// a hearing threshold level can either be a valid integer
// value in dB at a specific frequency and side or it can be an error code combined with a recommended
// outcome
// the characteristics for a given measurement are:
// frequency label string
// threshold level integer (can be null but there must be an error code)
// side left or right string
// error code string  (default no error or null)
// error code label string (default null)
// outcome string (default null)
//
QMap<QString,QString> HearingMeasurement::initCodeLookup()
{
    QMap<QString,QString> map;
    map["AA"] = "NOT_TESTED";
    map["DD"] = "DELETED";
    map["EA"] = "CONTRALATERAL_RECORDED";
    map["EB"] = "BASELINE_SHIFT";
    map["EC"] = "ADJACENT_FREQ";
    map["ED"] = "OUT_OF_RANGE";
    map["EE"] = "NO_RESPONSE";
    map["EF"] = "NO_THRESHOLD";
    map["E1"] = "NO_RESPONSE_1K";
    map["E2"] = "NO_THRESHOLD_1K";
    map["E3"] = "VERIFY_FAILED_1K";
    map["E4"] = "HANDSWITCH_ERROR";
    map["E5"] = "RESPONSE_NO_TONE";
    map["E6"] = "NO_THRESHOLD_AGAIN";
    map["E7"] = "TOO_MANY_FAILURES";
    map["E8"] = "EQUIPMENT_ERROR";
    return map;
}

QMap<QString,QString> HearingMeasurement::initOutcomeLookup()
{
    QMap<QString,QString> map;
    map["NOT_TESTED"] = "RUN_TEST";
    map["DELETED"] = "NONE";
    map["CONTRALATERAL_RECORDED"] = "RERUN_TEST";
    map["BASELINE_SHIFT"] = "RERUN_TEST";
    map["ADJACENT_FREQ"] = "RERUN_TEST";
    map["OUT_OF_RANGE"] = "RERUN_TEST";
    map["NO_RESPONSE"] = "RERUN_TEST";
    map["NO_THRESHOLD"] = "RERUN_TEST";
    map["NO_RESPONSE_1K"] = "REINSTRUCT_SUBJECT";
    map["NO_THRESHOLD_1K"] = "REINSTRUCT_SUBJECT";
    map["VERIFY_FAILED_1K"] = "REINSTRUCT_SUBJECT";
    map["HANDSWITCH_ERROR"] = "REINSTRUCT_SUBJECT";
    map["RESPONSE_NO_TONE"] = "REINSTRUCT_SUBJECT";
    map["NO_THRESHOLD_AGAIN"] = "REINSTRUCT_SUBJECT";
    map["TOO_MANY_FAILURES"] = "REINSTRUCT_SUBJECT";
    map["EQUIPMENT_ERROR"] = "CONTACT_SERVICE";
    return map;
}

QMap<int,QString> HearingMeasurement::initFrequencyLookup()
{
    QMap<int,QString> map;
    map[0] = "1 kHz Test";
    map[1] = "500 Hz";
    map[2] = "1 kHz";
    map[3] = "2 kHz";
    map[4] = "3 kHz";
    map[5] = "4 kHz";
    map[6] = "6 kHz";
    map[7] = "8 kHz";
    return map;
}

HearingMeasurement::HearingMeasurement(
        const QString &side,
        const int &index,
        const QString &code)
{
  fromCode(side,index,code);
}

// index is the position 0-7 in the returned HTL string from
// the RA300 RS232 port data
//
void HearingMeasurement::fromCode(const QString &side, const int &index, const QString &code)
{
    reset();
    if(frequencyLookup.contains(index))
    {
      setAttribute("side",Measurement::Value(side.toLower()));
      setAttribute("test",Measurement::Value(frequencyLookup[index]));
      if(codeLookup.contains(code))
      {
        QString err = codeLookup[code];
        setAttribute("error",Measurement::Value(err));
        setAttribute("outcome",Measurement::Value(outcomeLookup[err]));
      }
      else
      {
        QRegExp r("\\d*");
        if(r.exactMatch(code))
        {
          setAttribute("level",Measurement::Value(code.toInt(),"dB"));
        }
      }
    }
}

bool HearingMeasurement::isValid() const
{
    // side test: level(units) OR error (outcome)
    bool ok =
      hasAttribute("side") &&
      hasAttribute("test") &&
      (
        hasAttribute("level") ||
        (
          hasAttribute("error") &&
          hasAttribute("outcome")
        )
      );
    return ok;
}

QString HearingMeasurement::toString() const
{
  QString s;
  if(isValid())
  {
    s = QString("%1 %2").arg(
      getAttribute("side").toString(),
      getAttribute("test").toString());

    if(hasAttribute("level"))
    {
      s = QString("%1 %2").arg(s,getAttribute("level").toString());
    }
    else
    {
      s = QString("%1 %2 %3").arg(s,
          getAttribute("error").toString(),
          getAttribute("outcome").toString());
    }
  }
  return s;
}

QDebug operator<<(QDebug dbg, const HearingMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Hearing Measurement()";
    else
        dbg.nospace() << "Hearing Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
