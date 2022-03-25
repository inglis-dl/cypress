#ifndef ONYXOUTXML_H
#define ONYXOUTXML_H

#include <QXmlStreamReader>
#include <QString>

#include "OutDataModel.h"

class OnyxOutXml
{
public:
	static OutDataModel readImportantValues(const QString &transferOutPath, const QString& barcode);
private:
	static void skipToEndElement(QXmlStreamReader* reader, const QString &name);
	static QString readCommand(QXmlStreamReader* reader, OutDataModel* outData);
	static void readPatients(QXmlStreamReader* reader, OutDataModel* outData, const QString& patientId);
	static void readPatient(QXmlStreamReader* reader, OutDataModel* outData);
	static void readIntervals(QXmlStreamReader* reader, OutDataModel* outData);
	static void readInterval(QXmlStreamReader* reader, OutDataModel* outData);
	static void readTests(QXmlStreamReader* reader, OutDataModel* outData);
	static void readFVCTest(QXmlStreamReader* reader, OutDataModel* outData);
	static void readPatientDataAtTestTime(QXmlStreamReader* reader, OutDataModel* outData);
	static void readTrials(QXmlStreamReader* reader, OutDataModel* outData);
	static void readTrial(QXmlStreamReader* reader, OutDataModel* outData);
	static ResultParametersModel readResultParameters(QXmlStreamReader* reader, const QString& closingTagName);
	static ResultParameterModel readResultParameter(QXmlStreamReader* reader);
	static void readChannelFlow(QXmlStreamReader* reader, TrialDataModel* outData);
	static void readChannelVolume(QXmlStreamReader* reader, TrialDataModel* outData);
};

#endif // ONYXOUTXML_H
