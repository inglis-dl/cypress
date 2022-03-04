#pragma once

#include <QXmlStreamReader>
#include <QString>

#include "Models/OutDataModel.h"
#include "Models/TrialDataModel.h"

class OnyxOutXml
{
public:
	static OutDataModel ReadImportantValues();
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

