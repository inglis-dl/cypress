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
    QStringList list = s.split(",");
    if(15 == list.size())
    {
        int code = list.at(0).toInt();
        if(ChoiceReactionMeasurement::TEST_CODE == code)
        {
            setAttribute("screen_id",list.at(7));
            setAttribute("response_correct", list.at(8).toUInt());
            setAttribute("elapsed_time",list.at(9).toUInt(),"ms");
            setAttribute("correct_position",list.at(10).toLower());
            setAttribute("response_stimulus_interval",list.at(11).toUInt(),"ms");
        }
    }
}

bool ChoiceReactionMeasurement::isValid() const
{
    bool ok =
      hasAttribute("response_correct") &&
      hasAttribute("elapsed_time") &&
      hasAttribute("correct_position") &&
      hasAttribute("response_stimulus_interval");
    return ok;
}

QString ChoiceReactionMeasurement::toString() const
{
  QString s;
  if(isValid())
  {
    s = getAttribute("correct_position").toString() +
        (getAttributeValue("response_correct").toBool() ? " correct, time: " : " incorrect, time: ") +
         getAttribute("elapsed_time").toString() + ", interval: " +
         getAttribute("response_stimulus_interval").toString();
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

    m.setAttribute("screen_id",
      QString("RT%1").arg(id+7,2,10,QLatin1Char('0')));
    m.setAttribute("response_correct", true);
    m.setAttribute("elapsed_time",time,"ms");
    m.setAttribute("correct_position",(0 == (id % 2) ? "left" : "right"));
    m.setAttribute("response_stimulus_interval",stimulus_interval,"ms");

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
