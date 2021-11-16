#include "OnyxInXml.h"

void OnyxInXml::Write()
{
	//QString xmlData;
	QFile file("C:/ProgramData/ndd/Easy on-PC/OnyxIn.xml");
	if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false) return;
	
	QXmlStreamWriter stream(&file);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();
	//stream.writeAttribute("encoding", "utf-16");

	StartNddElement(&stream);
	AddCommand(&stream);
	AddPatients(&stream);
	stream.writeEndElement();
	stream.writeEndDocument();

	file.close();

	//qDebug() << xmlData << endl;
}

void OnyxInXml::AddParameter(QXmlStreamWriter* stream, QString name, QString text)
{
	stream->writeStartElement("Parameter");
	stream->writeAttribute("Name", name);
	stream->writeCharacters(text);
	stream->writeEndElement();
}

void OnyxInXml::StartNddElement(QXmlStreamWriter* stream)
{
	stream->writeStartElement("ndd");
	stream->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	stream->writeAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	stream->writeAttribute("Version", "ndd.EasyWarePro.V1");
}

void OnyxInXml::AddCommand(QXmlStreamWriter* stream)
{
	stream->writeStartElement("Command");
	stream->writeAttribute("Type", "PerformTest");
	AddParameter(stream, "OrderID", "1");
	AddParameter(stream, "TestType", "FVC");
	stream->writeEndElement();
}

void OnyxInXml::AddPatients(QXmlStreamWriter* stream)
{
	stream->writeStartElement("Patients");

	stream->writeStartElement("Patient");
	stream->writeAttribute("ID", "ONYX");
	stream->writeEmptyElement("LastName");
	stream->writeEmptyElement("FirstName");
	stream->writeTextElement("IsBioCal", "false");
	AddPatientDataAtPresent(stream);
	stream->writeEndElement();

	stream->writeEndElement();
}

void OnyxInXml::AddPatientDataAtPresent(QXmlStreamWriter* stream)
{
	stream->writeStartElement("PatientDataAtPresent");
	stream->writeTextElement("Gender", "Male");
	stream->writeTextElement("DateOfBirth", "1994-09-25");
	stream->writeTextElement("ComputedDateOfBirth", "false");
	stream->writeTextElement("Height", "1.8");
	stream->writeTextElement("Weight", "109");
	//stream->writeTextElement("Ethnicity", "Caucasian");
	stream->writeTextElement("Smoker", "false");
	stream->writeTextElement("Asthma", "false");
	stream->writeTextElement("COPD", "false");
	stream->writeEndElement();
}
