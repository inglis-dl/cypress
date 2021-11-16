#include <QXmlStreamWriter>
#include <QDebug>
#include <QString>
#include <QFile>

class OnyxInXml
{

public:
	static void Write();
private:
	static void StartNddElement(QXmlStreamWriter* stream);
	static void AddCommand(QXmlStreamWriter* stream);
	static void AddParameter(QXmlStreamWriter* stream, QString name, QString text);
	static void AddPatients(QXmlStreamWriter* stream);
	static void AddPatientDataAtPresent(QXmlStreamWriter* stream);
};