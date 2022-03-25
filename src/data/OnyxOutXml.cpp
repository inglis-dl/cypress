#include "OnyxOutXml.h"

#include <QDebug>
#include <QFile>

OutDataModel OnyxOutXml::readImportantValues(const QString& transferOutPath, const QString& barcode)
{
    OutDataModel outData;
    QFile file(transferOutPath);
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << "Cannot read file" << file.errorString();

        //TODO: should not be calling exit here!
        exit(0);
    }

    QXmlStreamReader reader(&file);
    bool next = reader.readNextStartElement();
    qDebug() << "ReadNextStartElement: " << next;
    qDebug() << "(TOP)Name: " << reader.name();
    if(next && "ndd" == reader.name())
    {
        QStringRef name;
        while(reader.readNextStartElement())
        {
            name = reader.name();
            qDebug() << "(Top2)Name: " << name;
            if("Command" == name)
            {
                QString commandType = readCommand(&reader, &outData);
                qDebug() << "commandType: " << commandType;
                // TODO: do something if commandType != "TestResult"
            }
            else if("Patients" == name)
            {
                //readPatients(&reader, &outData, "ONYX");
                readPatients(&reader, &outData, barcode);
            }
            else
            {
                skipToEndElement(&reader, name.toString());
            }
        }
    }
    file.close();
    return outData;
}

void OnyxOutXml::skipToEndElement(QXmlStreamReader* reader, const QString& name)
{
    qDebug() << "Skip";
    if(reader->isEndElement() && reader->name() == name)
    {
        qDebug() << "Already at end Skip on: " << reader->name();
        return;
    } 

    while(reader->readNext())
    {
        if(reader->isEndElement() && reader->name() == name)
        {
            qDebug() << "finishing Skip on: " << reader->name();
            return;
        }
    }
}

QString OnyxOutXml::readCommand(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    QStringRef type = reader->attributes().value("Type");
    qDebug() << "Name: " << name;
    qDebug() << "Type Attribute: " << type;
    while(reader->readNext())
    {
        name = reader->name();
        qDebug() << "Name: " << name;
        if(reader->isEndElement() && "Command" == name)
        {
            break;
        }
        else if(reader->isStartElement() && "Parameter" == name)
        {
            QStringRef nameAttribute = reader->attributes().value("Name");
            if("Attachment" == nameAttribute)
            {
                QString pdfPath = reader->readElementText();
                qDebug() << "Pdf path: " << pdfPath;
                if(QFile::exists(pdfPath))
                {
                    outData->pdfPath = pdfPath;
                }
            }
        }
    }
    return type.toString();
}

void OnyxOutXml::readPatients(QXmlStreamReader* reader, OutDataModel* outData, const QString& patientId)
{
    QStringRef name = reader->name();
    QStringRef id;
    qDebug() << "(Patients)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        id = reader->attributes().value("ID");
        qDebug() << "(Patients2)Name: " << name;
        qDebug() << "Id Attribute: " << id;
        if("Patient" == name && patientId == id)
        {
            readPatient(reader, outData);
            break;
        }
        else
        {
            skipToEndElement(reader, name.toString());
        }
    }

    skipToEndElement(reader, "Patients");
}

void OnyxOutXml::readPatient(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(Patient)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        qDebug() << "(Patient2)Name: " << name;
        if("Intervals" == name)
        {
            readIntervals(reader, outData);
        }
        else
        {
            skipToEndElement(reader, name.toString());
        }
    }
    skipToEndElement(reader, "Patient");
}

void OnyxOutXml::readIntervals(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    int numIntervals = 0;
    qDebug() << "(Intervals)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        qDebug() << "(Intervals2)Name: " << name;
        if("Interval" == name)
        {
            if(0 == numIntervals)
            {
                readInterval(reader, outData);
                numIntervals++;
            }
            else
            {
                qDebug() << "(Intervals2)Warning mulptiple intervals: " << numIntervals;
            }
        }
        else
        {
            skipToEndElement(reader, name.toString());
        }
    }
    skipToEndElement(reader, "Intervals");
}

void OnyxOutXml::readInterval(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(Interval)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        qDebug() << "(Interval2)Name: " << name;
        if("Tests" == name)
        {
            readTests(reader, outData);
        }
        else
        {
            skipToEndElement(reader, name.toString());
        }
    }
    skipToEndElement(reader, "Interval");
}

void OnyxOutXml::readTests(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(Tests)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        qDebug() << "(Tests2)Name: " << name;
        if("Test" == name)
        {
            QString typeOfTest = reader->attributes().value("TypeOfTest").toString();
            qDebug() << "(Tests2)TypeOfTest: " << typeOfTest;
            if("FVC" == typeOfTest)
            {
                readFVCTest(reader, outData);
            }
        }
        else
        {
            skipToEndElement(reader, name.toString());
        }
    }
    skipToEndElement(reader, "Tests");
}

void OnyxOutXml::readFVCTest(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(FVC)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        qDebug() << "(FVC)Name: " << name;
        if("TestDate" == name)
        {
            QString element = reader->readElementText();
            qDebug() << "Test date stored mock " << element;
            outData->testDate = QDateTime::fromString(element, "yyyy-MM-dd'T'hh:mm:ss.z");
            qDebug() << outData->testDate;
        }
        else if("BestValues" == name)
        {
            // TODO: Finish this
            outData->bestValues = readResultParameters(reader, "BestValues");
            qDebug() << "Best values stored mock ";
        }
        else if("SWVersion" == name)
        {
            outData->softwareVersion = reader->readElementText();
        }
        else if("PatientDataAtTestTime" == name)
        {
            readPatientDataAtTestTime(reader, outData);
            continue;
        }
        else if("QualityGradeOriginal" == name)
        {
            outData->qualityGradeOriginal = reader->readElementText();
            
        }
        else if("QualityGrade" == name)
        {
            outData->qualityGrade = reader->readElementText();
        }
        else if("Trials" == name)
        {
            readTrials(reader, outData);
            continue;
        }
        skipToEndElement(reader, name.toString());
    }
    skipToEndElement(reader, "Test");
}

void OnyxOutXml::readPatientDataAtTestTime(QXmlStreamReader* reader, OutDataModel* outData)
{
    qDebug() << "PatientDataAtTestTime stored mock ";
    QStringRef name = reader->name();
    qDebug() << "(pData)Name: " << name;
    while(reader->readNextStartElement()) {
        name = reader->name();
        qDebug() << "(pData)Name: " << name;
        if(name == "Height") {
            outData->height = reader->readElementText().toDouble();
            skipToEndElement(reader, name.toString());
        }
        else if(name == "Weight") {
            outData->weight = reader->readElementText().toDouble();
            skipToEndElement(reader, name.toString());
        }
        else if(name == "Ethnicity") {
            outData->ethnicity = reader->readElementText();
            skipToEndElement(reader, name.toString());
        }
        else if(name == "Smoker") {
            outData->smoker = reader->readElementText();
            skipToEndElement(reader, name.toString());
        }
        else if(name == "COPD") {
            outData->copd = reader->readElementText();
            skipToEndElement(reader, name.toString());
        }
        else if(name == "Asthma") {
            outData->asthma = reader->readElementText();
            skipToEndElement(reader, name.toString());
        }
        else if(name == "Gender") {
            outData->gender = reader->readElementText();
            skipToEndElement(reader, name.toString());
        }
        else if(name == "DateOfBirth") {
            outData->birthDate = QDate::fromString(reader->readElementText(), "yyyy-MM-dd");
            skipToEndElement(reader, name.toString());
        }
        else {
            skipToEndElement(reader, name.toString());
        }
    }
    skipToEndElement(reader, "PatientDataAtTestTime");
}

void OnyxOutXml::readTrials(QXmlStreamReader* reader, OutDataModel* outData)
{
    QStringRef name = reader->name();
    qDebug() << "(Trials)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        qDebug() << "(Trials)Name: " << name;
        if(name == "Trial")
        {
            readTrial(reader, outData);
        }
    }
    skipToEndElement(reader, "Trials");
}

void OnyxOutXml::readTrial(QXmlStreamReader* reader, OutDataModel* outData)
{
    TrialDataModel trialData;
    QStringRef name = reader->name();
    qDebug() << "(Trial)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        qDebug() << "(Trial)Name: " << name;
        if(name == "Date")
        {
            QString element = reader->readElementText();
            qDebug() << "Test date stored mock " << element;
            trialData.date = QDateTime::fromString(element, "yyyy-MM-dd'T'hh:mm:ss.z");
        }
        else if(name == "Number")
        {
            trialData.number = reader->readElementText().toInt();
        }
        else if(name == "Rank")
        {
            trialData.rank = reader->readElementText().toInt();
        }
        else if(name == "RankOriginal")
        {
            trialData.rankOriginal = reader->readElementText().toInt();
        }
        else if(name == "Accepted")
        {
            trialData.accepted = reader->readElementText();
        }
        else if(name == "AcceptedOriginal")
        {
            trialData.acceptedOriginal = reader->readElementText();
        }
        else if(name == "ManualAmbientOverride")
        {
            trialData.manualAmbientOverride = reader->readElementText();
        }
        else if(name == "ResultParameters")
        {
            trialData.resultParameters = readResultParameters(reader, "ResultParameters");
            continue;
        }
        else if(name == "ChannelFlow")
        {
            readChannelFlow(reader, &trialData);
            continue;
        }
        else if(name == "ChannelVolume")
        {
            readChannelVolume(reader, &trialData);
            continue;
        }
        skipToEndElement(reader, name.toString());
    }
    outData->trials.append(trialData);
    skipToEndElement(reader, "Trial");
}

ResultParametersModel OnyxOutXml::readResultParameters(QXmlStreamReader* reader, const QString& closingTagName)
{
    ResultParametersModel parameters;
    QStringRef name;
    QStringRef id;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        id = reader->attributes().value("ID");
        if(name == "ResultParameter")
        {
            QString idStr = id.toString();
            qDebug() << "idStr = " << idStr;

            parameters.results[idStr] = readResultParameter(reader);
        }
    }
    skipToEndElement(reader, closingTagName);
    return parameters;
}

ResultParameterModel OnyxOutXml::readResultParameter(QXmlStreamReader* reader)
{
    ResultParameterModel resultParameter;
    QStringRef name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        if(name == "DataValue")
        {
            resultParameter.dataValue = reader->readElementText().toDouble();
        }
        else if(name == "Unit")
        {
            resultParameter.unit = reader->readElementText();
        }
        else if(name == "PredictedValue")
        {
            resultParameter.predictedValue = reader->readElementText().toDouble();
        }
        else if(name == "LLNormalValue")
        {
            resultParameter.llNormalValue = reader->readElementText().toDouble();
        }
        skipToEndElement(reader, name.toString());
    }
    skipToEndElement(reader, "ResultParameter");
    return resultParameter;
}

void OnyxOutXml::readChannelFlow(QXmlStreamReader* reader, TrialDataModel* trialData)
{
    QStringRef name = reader->name();
    qDebug() << "(flow)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        qDebug() << "(flow)Name: " << name;
        if(name == "SamplingInterval")
        {
            trialData->flowInterval = reader->readElementText().toDouble();
        }
        else if(name == "SamplingValues")
        {
            trialData->flowValues = reader->readElementText();
        }
        skipToEndElement(reader, name.toString());
    }
}

void OnyxOutXml::readChannelVolume(QXmlStreamReader* reader, TrialDataModel* trialData)
{
    QStringRef name = reader->name();
    qDebug() << "(volume)Name: " << name;
    while(reader->readNextStartElement())
    {
        name = reader->name();
        qDebug() << "(volume)Name: " << name;
        if(name == "SamplingInterval")
        {
            trialData->volumeInterval = reader->readElementText().toDouble();
        }
        else if(name == "SamplingValues")
        {
            trialData->volumeValues = reader->readElementText();
        }
        skipToEndElement(reader, name.toString());
    }
}
