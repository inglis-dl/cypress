#include "EMRPluginWriter.h"

#include <QDebug>
#include <QFile>
#include <QXmlStreamWriter>

// pre-validated input json from the spirometer manager
//
void EMRPluginWriter::setInputData(const QVariantMap& input)
{
    if(input.isEmpty())
    {
        qDebug() << "ERROR: empty json input";
        return;
    }
    m_input = input;
}

void EMRPluginWriter::write(const QString& fileName) const
{
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return;

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    addHeader(stream);
    addCommand(stream);
    addPatients(stream);

    stream.writeEndElement();
    stream.writeEndDocument();
    file.close();
}

void EMRPluginWriter::addParameter(QXmlStreamWriter &stream, const QString& name, const QString& text) const
{
    stream.writeStartElement("Parameter");
    stream.writeAttribute("Name", name);
    stream.writeCharacters(text);
    stream.writeEndElement();
}

void EMRPluginWriter::addHeader(QXmlStreamWriter& stream) const
{
    stream.writeStartElement("ndd");
    stream.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    stream.writeAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
    stream.writeAttribute("Version", "ndd.EasyWarePro.V1");
}

void EMRPluginWriter::addCommand(QXmlStreamWriter& stream) const
{
    stream.writeStartElement("Command");
    stream.writeAttribute("Type", "PerformTest");
    addParameter(stream, "OrderID", "1");
    addParameter(stream, "TestType", "FVC");
    stream.writeEndElement();
}

void EMRPluginWriter::addPatients(QXmlStreamWriter& stream) const
{
    stream.writeStartElement("Patients");

    stream.writeStartElement("Patient");
    stream.writeAttribute("ID", m_input["barcode"].toString());
    stream.writeEmptyElement("LastName");
    stream.writeEmptyElement("FirstName");
    stream.writeTextElement("IsBioCal", "false");

    addPatientDataAtPresent(stream);

    stream.writeEndElement();
    stream.writeEndElement();
}

void EMRPluginWriter::addPatientDataAtPresent(QXmlStreamWriter& stream) const
{
    stream.writeStartElement("PatientDataAtPresent");

    // enforce capitalized gender: eg., male => Male
    //
    QString gender = m_input["gender"].toString().toLower();
    gender = gender.at(0).toUpper() + gender.mid(1);
    stream.writeTextElement("Gender", gender);

    stream.writeTextElement("DateOfBirth", m_input["date_of_birth"].toDate().toString("yyyy-MM-dd"));
    stream.writeTextElement("ComputedDateOfBirth", "false");

    // TODO: check that the units are decimal m ?
    //
    stream.writeTextElement("Height", QString::number(m_input["height"].toDouble()));
    stream.writeTextElement("Weight", QString::number(m_input["weight"].toDouble()));

    //stream.writeTextElement("Ethnicity", m_ethnicity);

    stream.writeTextElement("Smoker", (m_input["smoker"].toBool() ? "Yes" : "No"));

    //stream.writeTextElement("Asthma", QString(m_asthma));
    stream.writeTextElement("Asthma", "No");

    //stream.writeTextElement("COPD", QString(m_copd));
    stream.writeTextElement("COPD", "No");

    stream.writeEndElement();
}

