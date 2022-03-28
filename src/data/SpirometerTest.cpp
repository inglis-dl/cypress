#include "SpirometerTest.h"

#include "../managers/EMRPluginHelper.h"

#include <QJsonArray>

const QString barcode = "barcode";
const QString outputHeight = "output_height";
const QString outputWeight = "output_weight";
const QString outputEthnicity = "output_ethnicity";
const QString outputAsthma = "output_asthma";
const QString outputSmoker = "output_smoker";
const QString outputCopd = "output_copd";
const QString outputQualityGrade = "output_quality_grade";
const QString resFvcPred2 = "res_fvc_pred2";
const QString resFvcLlnormal2 = "res_fvc_llnormal2";
const QString resFev1Pred2 = "res_fev1_pred2";
const QString resFev1Llnormal2 = "res_fev1_llnormal2";
const QString resFev1FvcPred2 = "res_fev1_fvc_pred2";
const QString resFev1FvcLlnormal2 = "res_fev1_fvc_llnormal2";

// the minimum output data keys required from a successful a test
//
SpirometerTest::SpirometerTest()
{
    m_outputKeyList << barcode;
    m_outputKeyList << outputHeight;
    m_outputKeyList << outputWeight;
    m_outputKeyList << outputEthnicity;
    m_outputKeyList << outputAsthma;
    m_outputKeyList << outputSmoker;
    m_outputKeyList << outputCopd;
    m_outputKeyList << outputQualityGrade;
    m_outputKeyList << resFvcPred2;
    m_outputKeyList << resFvcLlnormal2;
    m_outputKeyList << resFev1Pred2;
    m_outputKeyList << resFev1Llnormal2;
    m_outputKeyList << resFev1FvcPred2;
    m_outputKeyList << resFev1FvcLlnormal2;
}

void SpirometerTest::simulate(const QJsonObject &obj)
{
  Q_UNUSED(obj)
}

void SpirometerTest::fromFile(const QString& fileName)
{
    EMRPluginHelper helper;
    EMRPluginHelper::OutDataModel outData = helper.read(fileName);

    addMetaData(outputHeight, outData.height);
    addMetaData(outputWeight, outData.weight);
    addMetaData(outputEthnicity, outData.ethnicity);
    addMetaData(outputAsthma, outData.asthma);
    addMetaData(outputSmoker, outData.smoker);
    addMetaData(outputCopd, outData.copd);
    addMetaData(outputQualityGrade, outData.qualityGrade);
    addMetaData("pdfPath", outData.pdfPath);

    if(0 < outData.trials.count())
    {
        foreach(const auto trial, outData.trials)
        {
            SpirometerMeasurement measurement;
            measurement.fromTrialData(trial);
            m_measurementList.append(measurement);
        }

        EMRPluginHelper::ResultParametersModel metaResults = getMeasurement(0).getMetaResults();
        if(metaResults.results.contains("FVC"))
        {
            EMRPluginHelper::ResultParameterModel fvc = metaResults.results["FVC"];
            addMetaData(resFvcPred2, fvc.predictedValue);
            addMetaData(resFvcLlnormal2, fvc.llNormalValue);
        }
        if(metaResults.results.contains("FEV1"))
        {
            EMRPluginHelper::ResultParameterModel fev1 = metaResults.results["FEV1"];
            addMetaData(resFev1Pred2, fev1.predictedValue, fev1.unit);
            addMetaData(resFev1Llnormal2, fev1.llNormalValue, fev1.unit);
        }
        if(metaResults.results.contains("FEV1_FVC"))
        {
            EMRPluginHelper::ResultParameterModel fev1Fvc = metaResults.results["FEV1_FVC"];
            addMetaData(resFev1FvcPred2, fev1Fvc.predictedValue);
            addMetaData(resFev1FvcLlnormal2, fev1Fvc.llNormalValue);
        }
    }
}

// String representation for debug and GUI display purposes
//
QString SpirometerTest::toString() const
{
    QString displayString;
    if(isValid())
    {
        QStringList stringList;
        foreach( const auto measurement, m_measurementList)
        {
            stringList << measurement.toString();
        }
        displayString = stringList.join("\n");
    }
    return displayString;
}

QStringList SpirometerTest::getMeasurementHeaderValues() const
{
    QStringList headerValues;
    if(isValid())
    {
      headerValues = m_measurementList[0].getHeaderValues();
    }
    return headerValues;
}

QList<QStringList> SpirometerTest::getMeasurementsAsLists() const
{
    QList<QStringList> data;
    if(isValid())
    {
        data.append(getMeasurementHeaderValues());
        foreach(const auto measurement, m_measurementList)
        {
            data.append(measurement.toString().split(","));
        }
    }
    return data;
}

bool SpirometerTest::isValid() const
{
    return 0 < m_measurementList.count();
}

// String keys are converted to snake_case
//
QJsonObject SpirometerTest::toJsonObject() const
{
    QJsonArray jsonArr;
    foreach(const auto measurement, m_measurementList)
    {
      jsonArr.append(measurement.toJsonObject());
    }
    QJsonObject json;
    if(hasMetaData())
      json.insert("test_meta_data", m_metaData.toJsonObject());
    json.insert("test_results", jsonArr);
    return json;
}
