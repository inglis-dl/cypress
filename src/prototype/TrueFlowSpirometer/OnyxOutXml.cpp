#include "OnyxOutXml.h"

#include <QStringRef>
#include <QDebug>
#include <QFile>

OutDataModel OnyxOutXml::ReadImportantValues()
{
    OutDataModel outData;
    QFile file("C:/ProgramData/ndd/Easy on-PC/OnyxOutTest.xml");
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Cannot read file" << file.errorString();
        exit(0);
    }

    QXmlStreamReader reader(&file);
    bool next = reader.readNextStartElement();
    qDebug() << "ReadNextStartElement: " << next << endl;
    qDebug() << "(TOP)Name: " << reader.name() << endl;
    if (next && reader.name() == "ndd") {
        
        QStringRef name;
        while (reader.readNextStartElement()) {
            name = reader.name();
            qDebug() << "(Top2)Name: " << name << endl;
            if (name == "Command") {
                QString commandType = ReadCommand(&reader);
                // TODO: do something if commandType != "TestResult"
            }
            else if (name == "Patients") {
                ReadPatients(&reader, &outData, "ONYX");
            }
            else {
                SkipToEndElement(&reader, name.toString());
            }
        }
    }
    file.close();
    return outData;
}

void OnyxOutXml::SkipToEndElement(QXmlStreamReader* reader, QString name)
{
    qDebug() << "Skip" << endl;
    if (reader->isEndElement() && reader->name() == name) {
        qDebug() << "Already at end Skip on: " << reader->name() << endl;
        return;
    } 

    while (reader->readNext()) {
        if (reader->isEndElement() && reader->name() == name) {
            qDebug() << "finishing Skip on: " << reader->name() << endl;
            return;
        }
    }
}

QString OnyxOutXml::ReadCommand(QXmlStreamReader* reader)
{
    QStringRef name = reader->name();
    QStringRef type = reader->attributes().value("Type");
    qDebug() << "Name: " << name << endl;
    qDebug() << "Type Attribute: " << type << endl;
    while (reader->readNext()) {
        if (reader->isEndElement() && name == "Command") {
            break;
        }
    }
    return type.toString();
}

void OnyxOutXml::ReadPatients(QXmlStreamReader* reader, OutDataModel* outData, QString patientId)
{
    QStringRef name = reader->name();
    QStringRef id;
    qDebug() << "(Patients)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        id = reader->attributes().value("ID");
        qDebug() << "(Patients2)Name: " << name << endl;
        qDebug() << "Id Attribute: " << id << endl;
        if ( name == "Patient" && id == patientId) {
            ReadPatient(reader, outData);
            break;
        }
        else {
            SkipToEndElement(reader, name.toString());
        }
    }

    SkipToEndElement(reader, "Patients");
}

void OnyxOutXml::ReadPatient(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(Patient)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(Patient2)Name: " << name << endl;
        if (name == "Intervals") {
            ReadIntervals(reader, outData);
        }
        else {
            SkipToEndElement(reader, name.toString());
        }
    }
    SkipToEndElement(reader, "Patient");
}

void OnyxOutXml::ReadIntervals(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    int numIntervals = 0;
    qDebug() << "(Intervals)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(Intervals2)Name: " << name << endl;
        if (name == "Interval") {
            if (numIntervals == 0) {
                ReadInterval(reader, outData);
                numIntervals++;
            }
            else {
                qDebug() << "(Intervals2)Warning mulptiple intervals: " << numIntervals << endl;
            }
        }
        else {
            SkipToEndElement(reader, name.toString());
        }
    }
    SkipToEndElement(reader, "Intervals");
}

void OnyxOutXml::ReadInterval(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(Interval)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(Interval2)Name: " << name << endl;
        if (name == "Tests") {
            ReadTests(reader, outData);
        }
        else {
            SkipToEndElement(reader, name.toString());
        }
    }
    SkipToEndElement(reader, "Interval");
}

void OnyxOutXml::ReadTests(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(Tests)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(Tests2)Name: " << name << endl;
        if (name == "Test") {
            QString typeOfTest = reader->attributes().value("TypeOfTest").toString();
            qDebug() << "(Tests2)TypeOfTest: " << typeOfTest << endl;
            if (typeOfTest == "FVC") {
                ReadFVCTest(reader, outData);
            }
        }
        else {
            SkipToEndElement(reader, name.toString());
        }
    }
    SkipToEndElement(reader, "Tests");
}

void OnyxOutXml::ReadFVCTest(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(FVC)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(FVC)Name: " << name << endl;
        if (name == "TestDate") {
            QString element = reader->readElementText();
            qDebug() << "Test date stored mock " << element << endl;
            outData->testDate = QDateTime::fromString(element, "yyyy-MM-dd'T'hh:mm:ss.z");
            qDebug() << outData->testDate << endl;
        }
        else if (name == "BestValues") {
            // TODO: Finish this
            outData->bestValues = ReadResultParameters(reader, "BestValues");
            qDebug() << "Best values stored mock " << endl;
        }
        else if (name == "SWVersion") {
            outData->softwareVersion = reader->readElementText();
        }
        else if (name == "PatientDataAtTestTime") {
            ReadPatientDataAtTestTime(reader, outData);
            continue;
        }
        else if (name == "QualityGradeOriginal") {
            outData->qualityGradeOriginal = reader->readElementText();
            
        }
        else if (name == "QualityGrade") {
            outData->qualityGrade = reader->readElementText();
        }
        else if (name == "Trials") {
            ReadTrials(reader, outData);
            continue;
        }
        SkipToEndElement(reader, name.toString());
    }
    SkipToEndElement(reader, "Test");
}

void OnyxOutXml::ReadPatientDataAtTestTime(QXmlStreamReader* reader, OutDataModel* outData)
{
    qDebug() << "PatientDataAtTestTime stored mock " << endl;
    QStringRef name = reader->name();
    qDebug() << "(pData)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(pData)Name: " << name << endl;
        if (name == "Height") {
            outData->height = reader->readElementText().toDouble();
            SkipToEndElement(reader, name.toString());
        }
        else if (name == "Weight") {
            outData->weight = reader->readElementText().toDouble();
            SkipToEndElement(reader, name.toString());
        }
        else if (name == "Ethnicity") {
            outData->ethnicity = reader->readElementText();
            SkipToEndElement(reader, name.toString());
        }
        else if (name == "Smoker") {
            outData->smoker = reader->readElementText();
            SkipToEndElement(reader, name.toString());
        }
        else if (name == "COPD") {
            outData->copd = reader->readElementText();
            SkipToEndElement(reader, name.toString());
        }
        else if (name == "Asthma") {
            outData->asthma = reader->readElementText();
            SkipToEndElement(reader, name.toString());
        }
        else if (name == "Gender") {
            outData->gender = reader->readElementText();
            SkipToEndElement(reader, name.toString());
        }
        else if (name == "DateOfBirth") {
            outData->birthDate = QDate::fromString(reader->readElementText(), "yyyy-MM-dd");
            SkipToEndElement(reader, name.toString());
        }
        else {
            SkipToEndElement(reader, name.toString());
        }
    }
    SkipToEndElement(reader, "PatientDataAtTestTime");
}

void OnyxOutXml::ReadTrials(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(Trials)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(Trials)Name: " << name << endl;
        if (name == "Trial") {
            ReadTrial(reader, outData);
        }
    }
    SkipToEndElement(reader, "Trials");
}

void OnyxOutXml::ReadTrial(QXmlStreamReader* reader, OutDataModel* outData)
{
    TrialDataModel trialData;
    QStringRef name = reader->name();
    qDebug() << "(Trial)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(Trial)Name: " << name << endl;
        if (name == "Date") {
            QString element = reader->readElementText();
            qDebug() << "Test date stored mock " << element << endl;
            trialData.date = QDateTime::fromString(element, "yyyy-MM-dd'T'hh:mm:ss.z");
        }
        else if (name == "Number") {
            trialData.number = reader->readElementText().toInt();
        }
        else if (name == "Rank") {
            trialData.rank = reader->readElementText().toInt();
        }
        else if (name == "RankOriginal") {
            trialData.rankOriginal = reader->readElementText().toInt();
        }
        else if (name == "Accepted") {
            trialData.accepted = reader->readElementText();
        }
        else if (name == "AcceptedOriginal") {
            trialData.acceptedOriginal = reader->readElementText();
        }
        else if (name == "ManualAmbientOverride") {
            trialData.manualAmbientOverride = reader->readElementText();
        }
        else if (name == "ResultParameters") {
            trialData.resultParameters = ReadResultParameters(reader, "ResultParameters");
            continue;
        }
        else if (name == "ChannelFlow") {
            ReadChannelFlow(reader, &trialData);
            continue;
        }
        else if (name == "ChannelVolume") {
            ReadChannelVolume(reader, &trialData);
            continue;
        }
        SkipToEndElement(reader, name.toString());
    }
    outData->trials.append(trialData);
    SkipToEndElement(reader, "Trial");
}

ResultParametersModel OnyxOutXml::ReadResultParameters(QXmlStreamReader* reader,  QString closingTagName)
{
    ResultParametersModel parameters;
    QStringRef name;
    QStringRef id;
    while (reader->readNextStartElement()) {
        name = reader->name();
        id = reader->attributes().value("ID");
        if (name == "ResultParameter") {
            QString idStr = id.toString();
            qDebug() << "idStr = " << idStr << endl;

            parameters.results[idStr] = ReadResultParameter(reader);
        }
    }
    SkipToEndElement(reader, closingTagName);
    return parameters;
}

ResultParameterModel OnyxOutXml::ReadResultParameter(QXmlStreamReader* reader)
{
    ResultParameterModel resultParameter;
    QStringRef name;
    while (reader->readNextStartElement()) {
        name = reader->name();
        if (name == "DataValue") {
            resultParameter._dataValue = reader->readElementText().toDouble();
        }
        else if (name == "Unit") {
            resultParameter._unit = reader->readElementText();
        }
        else if (name == "PredictedValue") {
            resultParameter._predictedValue = reader->readElementText().toDouble();
        }
        else if (name == "LLNormalValue") {
            resultParameter._llNormalValue = reader->readElementText().toDouble();
        }
        SkipToEndElement(reader, name.toString());
    }
    SkipToEndElement(reader, "ResultParameter");
    return resultParameter;
}

void OnyxOutXml::ReadChannelFlow(QXmlStreamReader* reader, TrialDataModel* trialData)
{
    QStringRef name = reader->name();
    qDebug() << "(flow)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(flow)Name: " << name << endl;
        if (name == "SamplingInterval") {
            trialData->flowInterval = reader->readElementText().toDouble();
        }
        else if (name == "SamplingValues") {
            QList<QString> strValues = reader->readElementText().split(" ");
            for (int i = 0; i < strValues.count(); i++) {
                trialData->flowValues.append(strValues[i].toDouble());
            }
        }
        SkipToEndElement(reader, name.toString());
    }
}

void OnyxOutXml::ReadChannelVolume(QXmlStreamReader* reader, TrialDataModel* trialData)
{
    QStringRef name = reader->name();
    qDebug() << "(volume)Name: " << name << endl;
    while (reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(volume)Name: " << name << endl;
        if (name == "SamplingInterval") {
            trialData->volumeInterval = reader->readElementText().toDouble();
        }
        else if (name == "SamplingValues") {
            QList<QString> strValues = reader->readElementText().split(" ");
            for (int i = 0; i < strValues.count(); i++) {
                trialData->volumeValues.append(strValues[i].toDouble());
            }
        }
        SkipToEndElement(reader, name.toString());
    }
}
