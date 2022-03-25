#include <QXmlStreamWriter>
#include <QString>
#include <QDate>

class OnyxInXml
{

public:
	void setParticipantInfo(const QString& barcode, const QString& gender, const QDate dateOfBirth, const double height, const double& weight, const bool& smoker);
	void write(const QString& dirPath) const;
private:
	void startNddElement(QXmlStreamWriter &stream) const;
	void addCommand(QXmlStreamWriter &stream) const;
	void addParameter(QXmlStreamWriter &stream, const QString& name, const QString & text) const;
	void addPatients(QXmlStreamWriter &stream) const;
	void addPatientDataAtPresent(QXmlStreamWriter &stream) const;

	QString m_barcode;
	QString m_gender;
	QDate m_dateOfBirth;
	double m_height;
	double m_weight;
	//QString m_ethnicity;
	bool m_smoker;
	//bool m_asthma;
	//bool m_copd;
};