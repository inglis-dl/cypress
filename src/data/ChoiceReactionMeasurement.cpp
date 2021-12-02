#include "ChoiceReactionMeasurement.h"

#include <QDebug>
#include <QRandomGenerator>

/**
 * sample input
 * lines start with and initial code
 * first line is the test code - 1
 * all subsequent lines that contain actual data start with the test code (11 for Choice Reaction Test)
 * last line is the test code + 1
 *
 * 10,Test,Version,Clinic,InterviewerId,UserId,UserStartDateTime,ScreenId,IfCorrect1Else0,ElapsedTime_ms,CorrectAnswer,ResponseStimulusInterval_ms,Empty,Empty,Dummy
 * 11,Reaction,vC1.3.3t,Default,None,12345678,11/17/2021 12:26:54 PM,RT08,1,370,Right,350,,,9
 *  0         1       2       3    4         5                      6   7 8   9     10  11 12 13 14
 * 12,Reaction,vC1.3.3t,Default,None,12345678,11/17/2021 12:26:54 PM,Level,1,EndDateTime,11/17/2021 12:28:02 PM,StartDateTimes,11/17/2021 12:26:54 PM,11/17/2021 12:27:10 PM,69
 *
 */

// Different tests that are part of the Cardiff Cognitive Battery
// are identified in the output by integer code.  The choice
// reaction test has code = 11
//
const int ChoiceReactionMeasurement::TEST_CODE = 11;

void ChoiceReactionMeasurement::fromString(const QString &s)
{
    // parse the comma deliminated string
    QStringList l = s.split(",");
    if(15 == l.size())
    {
        int code = l.at(0).toInt();
        if(ChoiceReactionMeasurement::TEST_CODE == code)
        {
            setCharacteristic("screen id",l.at(7));
            setCharacteristic("response correct", l.at(8).toUInt());
            setCharacteristic("elapsed time",l.at(9).toUInt());
            setCharacteristic("units","ms");
            setCharacteristic("correct position",l.at(10).toLower());
            setCharacteristic("response stimulus interval",l.at(11).toUInt());
        }
    }
}

bool ChoiceReactionMeasurement::isValid() const
{
    bool ok =
            hasCharacteristic("response correct") &&
            hasCharacteristic("elapsed time") &&
            hasCharacteristic("units") &&
            hasCharacteristic("correct position") &&
            hasCharacteristic("response stimulus interval");
    return ok;
}

QString ChoiceReactionMeasurement::toString() const
{
  QString s;
  if(isValid())
  {
    s = getCharacteristic("correct position").toString() +
        (getCharacteristic("response correct").toBool() ? " correct, time: " : " incorrect, time: ") +
         QString::number(getCharacteristic("elapsed time").toUInt()) +
        "(" + getCharacteristic("units").toString() + "), interval: " +
         QString::number(getCharacteristic("response stimulus interval").toUInt()) +
        "(" + getCharacteristic("units").toString() + ")";
  }
  return s;
}

ChoiceReactionMeasurement ChoiceReactionMeasurement::simulate()
{
    ChoiceReactionMeasurement m;
    static int id = 1;

    // the response stimuluw is always a multiple of 50 ms
    int stimulus_interval = QRandomGenerator::global()->bounded(1,15)*50;

    // create a simulated time reaction based on the interval
    int low = qMax(stimulus_interval - 50, 40);
    int high = stimulus_interval + 200;
    int time = QRandomGenerator::global()->bounded(low,high);

    m.setCharacteristic("screen id",
      "RT" + QStringLiteral("%1").arg(id+7,2,10,QLatin1Char('0')));
    m.setCharacteristic("response correct", 1);
    m.setCharacteristic("elapsed time",time);
    m.setCharacteristic("units","ms");
    m.setCharacteristic("correct position",(0 == (id % 2) ? "left" : "right"));
    m.setCharacteristic("response stimulus interval",stimulus_interval);

    if(60 < id++) id = 1;

    return m;
}

QDebug operator<<(QDebug dbg, const ChoiceReactionMeasurement &item)
{
    const QString s = item.toString();
    if (s.isEmpty())
        dbg.nospace() << "Choice Reaction Measurement()";
    else
        dbg.nospace() << "Choice Reaction Measurement(" << s << " ...)";
    return dbg.maybeSpace();
}
