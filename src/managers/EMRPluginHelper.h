#ifndef EMRPLUGINHELPER_H
#define EMRPLUGINHELPER_H

#include <QJsonObject>

QT_FORWARD_DECLARE_CLASS(QXmlStreamWriter)
QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)

class EMRPluginHelper
{
public:
    EMRPluginHelper() = default;

    class ResultParameterModel
    {
      public:
        double dataValue;
        QString unit;
        double predictedValue;
        double llNormalValue;
    };

    class ResultParametersModel
    {
      public:
        QMap<QString, ResultParameterModel> results;
    };

    class TrialDataModel
    {
      public:
        QDateTime date;
        int number;
        int rank;
        int rankOriginal;
        QString accepted;
        QString acceptedOriginal;
        QString manualAmbientOverride;
        ResultParametersModel resultParameters;
        double flowInterval = -1;
        QString flowValues;
        double volumeInterval = -1;
        QString volumeValues;
    };

    class OutDataModel
    {
      public:
        QString id = "";
        double height = -1;
        double weight = -1;
        QString ethnicity = "";
        QString asthma = "";
        QString smoker = "";
        QString copd = "";
        QString gender = "";
        QDate birthDate;
        QDateTime testDate;
        ResultParametersModel bestValues;
        QString softwareVersion = "";
        QString qualityGrade = "";
        QString qualityGradeOriginal = "";
        QList<TrialDataModel> trials;
        QString pdfPath = "";
    };

    void setInputData(const QJsonObject&);

    void write(const QString&) const;

    OutDataModel read(const QString&);

private:

    QJsonObject m_input;

    void addHeader(QXmlStreamWriter&) const;
    void addCommand(QXmlStreamWriter&) const;
    void addParameter(QXmlStreamWriter&, const QString&, const QString&) const;
    void addPatients(QXmlStreamWriter&) const;
    void addPatientDataAtPresent(QXmlStreamWriter&) const;

    void skipToEndElement(QXmlStreamReader*, const QString&);
    QString readCommand(QXmlStreamReader*, OutDataModel*);
    void readPatients(QXmlStreamReader*, OutDataModel*);
    void readPatient(QXmlStreamReader*, OutDataModel*);
    void readIntervals(QXmlStreamReader*, OutDataModel*);
    void readInterval(QXmlStreamReader*, OutDataModel*);
    void readTests(QXmlStreamReader*, OutDataModel*);
    void readFVCTest(QXmlStreamReader*, OutDataModel*);
    void readPatientDataAtTestTime(QXmlStreamReader*, OutDataModel*);
    void readTrials(QXmlStreamReader*, OutDataModel*);
    void readTrial(QXmlStreamReader*, OutDataModel*);
    ResultParametersModel readResultParameters(QXmlStreamReader*, const QString&);
    ResultParameterModel readResultParameter(QXmlStreamReader*);
    void readChannelFlow(QXmlStreamReader*, TrialDataModel*);
    void readChannelVolume(QXmlStreamReader*, TrialDataModel*);

};

#endif // EMRPLUGINHELPER_H
