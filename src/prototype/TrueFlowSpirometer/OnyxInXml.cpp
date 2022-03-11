#include "OnyxInXml.h"

#include <QFile>
#include <QDebug>

void OnyxInXml::setParticipantInfo(const QString& gender, const QDate dateOfBirth, const double height, const double& weight, const bool& smoker)
{
	m_gender = gender;
	m_dateOfBirth = dateOfBirth;
	m_height = height;
	m_weight = weight;
	//m_ethnicity = ethnicity;
	m_smoker = smoker;
	//m_asthma = asthma;
	//m_copd = copd;
}

void OnyxInXml::write(const QString & transferInFilePath) const
{
	//"C:/ProgramData/ndd/Easy on-PC/OnyxIn.xml"
	QFile file(transferInFilePath);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false) return;
	
	QXmlStreamWriter stream(&file);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();
	//stream.writeAttribute("encoding", "utf-16");

	startNddElement(stream);
	addCommand(stream);
	addPatients(stream);
	stream.writeEndElement();
	stream.writeEndDocument();

	file.close();
}

void OnyxInXml::addParameter(QXmlStreamWriter &stream, const QString& name, const QString& text) const
{
	stream.writeStartElement("Parameter");
	stream.writeAttribute("Name", name);
	stream.writeCharacters(text);
	stream.writeEndElement();
}

void OnyxInXml::startNddElement(QXmlStreamWriter& stream) const
{
	stream.writeStartElement("ndd");
	stream.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	stream.writeAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	stream.writeAttribute("Version", "ndd.EasyWarePro.V1");
}

void OnyxInXml::addCommand(QXmlStreamWriter& stream) const
{
	stream.writeStartElement("Command");
	stream.writeAttribute("Type", "PerformTest");
	addParameter(stream, "OrderID", "1");
	addParameter(stream, "TestType", "FVC");
	stream.writeEndElement();
}

void OnyxInXml::addPatients(QXmlStreamWriter& stream) const
{
	stream.writeStartElement("Patients");

	stream.writeStartElement("Patient");
	stream.writeAttribute("ID", "ONYX");

	//stream.writeEmptyElement("LastName");
	//stream.writeEmptyElement("FirstName");
	stream.writeTextElement("LastName", "91827364");
	stream.writeTextElement("FirstName", "clsa");

	stream.writeTextElement("IsBioCal", "false");
	addPatientDataAtPresent(stream);
	stream.writeEndElement();

	stream.writeEndElement();
}

void OnyxInXml::addPatientDataAtPresent(QXmlStreamWriter& stream) const
{
	stream.writeStartElement("PatientDataAtPresent");
	stream.writeTextElement("Gender", m_gender);
	stream.writeTextElement("DateOfBirth", m_dateOfBirth.toString("yyyy-MM-dd"));
	stream.writeTextElement("ComputedDateOfBirth", "false");
	stream.writeTextElement("Height", QString::number(m_height));
	stream.writeTextElement("Weight", QString::number(m_weight));
	//stream.writeTextElement("Ethnicity", m_ethnicity);
	//stream.writeTextElement("Ethnicity", m_ethnicity);
	stream.writeTextElement("Smoker", QString(m_smoker));
	//stream.writeTextElement("Asthma", QString(m_asthma));
	stream.writeTextElement("Asthma", "false");
	//stream.writeTextElement("COPD", QString(m_copd));
	stream.writeTextElement("COPD", "false");
	stream.writeEndElement();
}
