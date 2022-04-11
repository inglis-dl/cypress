#ifndef EMRPLUGINWRITER_H
#define EMRPLUGINWRITER_H

#include <QJsonObject>

QT_FORWARD_DECLARE_CLASS(QXmlStreamWriter)

class EMRPluginWriter
{
public:
    EMRPluginWriter() = default;

    void setInputData(const QVariantMap&);

    void write(const QString&) const;

private:

    QVariantMap m_input;

    void addHeader(QXmlStreamWriter&) const;
    void addCommand(QXmlStreamWriter&) const;
    void addParameter(QXmlStreamWriter&, const QString&, const QString&) const;
    void addPatients(QXmlStreamWriter&) const;
    void addPatientDataAtPresent(QXmlStreamWriter&) const;
};

#endif // EMRPLUGINWRITER_H
