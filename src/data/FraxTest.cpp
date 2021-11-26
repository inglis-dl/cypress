#include "FraxTest.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

void FraxTest::fromFile(const QString &fileName)
{
    QFile ifile(fileName);
    if (ifile.open(QIODevice::ReadOnly))
    {
        qDebug() << "OK, reading input file " << fileName;

        QTextStream instream(&ifile);
        QString fraxContent = instream.readLine();
        if (instream.atEnd() == false) {
            qDebug() << "Frax: More lines of content than expected";
        }

        qDebug() << "closed stream";
        ifile.close();

        FraxMeasurement fraxMeasurement;
        fraxMeasurement.fromString(fraxContent);
        addMeasurement(fraxMeasurement);
        qDebug() << "found " << (fraxMeasurement.isValid() ? "VALID" : "INVALID") << "measurement item positions at line ";
        qDebug() << fraxMeasurement.toString();
    }
}

QString FraxTest::toString() const
{
    QString outStr;
    if (isValid())
    {
        QStringList tempList;
        for (auto&& measurement : m_measurementList)
        {
            tempList << measurement.toString();
        }
        outStr = tempList.join("\n");
    }
    return outStr;
}

bool FraxTest::isValid() const
{
    bool okTest = false;
    bool onlyOneMeasurment = getNumberOfMeasurements() == 1;
    if (onlyOneMeasurment) {
        okTest = getMeasurement(0).isValid();
    }

    qDebug() << "test results " << (okTest?"OK":"ERROR");
    return  okTest;
}

QJsonObject FraxTest::toJsonObject() const
{
    QJsonArray jsonArr;
    for (auto&& x : m_measurementList)
    {
        QJsonObject test = x.toJsonObject();
        jsonArr.append(test);
    }
    QJsonObject json;
    json.insert("test_results", jsonArr);
    return json;
}
