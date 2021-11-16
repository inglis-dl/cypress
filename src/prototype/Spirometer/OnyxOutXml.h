#pragma once

#include <QXmlStreamReader>
#include <QFile>
#include <QString>
#include <QStringRef>
#include <QDebug>

#include "OutDataModel.h"
#include "TrialDataModel.h"

class OnyxOutXml
{
public:
	static void ReadImportantValues();
private:
	static void SkipToEndElement(QXmlStreamReader* reader, QString name);
	static QString ReadCommand(QXmlStreamReader* reader);
	static void ReadPatients(QXmlStreamReader* reader, OutDataModel* outData, QString patientId);
	static void ReadPatient(QXmlStreamReader* reader, OutDataModel* outData);
	static void ReadIntervals(QXmlStreamReader* reader, OutDataModel* outData);
	static void ReadInterval(QXmlStreamReader* reader, OutDataModel* outData);
	static void ReadTests(QXmlStreamReader* reader, OutDataModel* outData);
	static void ReadFVCTest(QXmlStreamReader* reader, OutDataModel* outData);
	static void ReadPatientDataAtTestTime(QXmlStreamReader* reader, OutDataModel* outData);
	static void ReadTrials(QXmlStreamReader* reader, OutDataModel* outData);
	static void ReadTrial(QXmlStreamReader* reader, OutDataModel* outData);
	static ResultParametersModel ReadResultParameters(QXmlStreamReader* reader, QString closingTagName);
	static ResultParameterModel ReadResultParameter(QXmlStreamReader* reader);
	static void ReadChannelFlow(QXmlStreamReader* reader, TrialDataModel* outData);
	static void ReadChannelVolume(QXmlStreamReader* reader, TrialDataModel* outData);
};

