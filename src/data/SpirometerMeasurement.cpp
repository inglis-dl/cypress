#include "SpirometerMeasurement.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QDateTime>
#include "../auxiliary/Utilities.h"

const QStringList SpirometerMeasurement::parameterList = {"DataValue","Unit","PredictedValue","LLNormalValue"};

const q_stringMap SpirometerMeasurement::channelMap = {
    {"SamplingInterval","sampling_interval"},
    {"SamplingValues","values"},
    {"TimeZeroOffset","time_zero_offset"},
    {"DefaultVTPlotDrawEndIdx","default_vt_plot_draw_end_index"}
};

const q_stringMap SpirometerMeasurement::resultMap = {
    {"AmbHumidity","ambient_humidity"},
    {"AmbPressure","ambient_pressure"},
    {"AmbTemp","ambient_temperature"},
    {"AmbTemp_Fahr","ambient_temperature_fahr"},
    {"BEV","bev"},
    {"BTPSex","btps_ex"},
    {"BTPSin","btps_in"},
    {"EOTV","eotv"},
    {"FEF10","fef_10"},
    {"FEF25","fef_25"},
    {"FEF2575","fef_25_75"},
    {"FEF2575_6","fef_25_75_6"},
    {"FEF2575_FVC","fef_25_75_fvc"},
    {"FEF40","fef_40"},
    {"FEF50","fef_50"},
    {"FEF50_FVC","fef_50_fvc"},
    {"FEF50_VCmax","fef_50_vcmax"},
    {"FEF60","fef_60"},
    {"FEF75","fef_75"},
    {"FEF7585","fef_75_85"},
    {"FEF80","fef_80"},
    {"FET","fet"},
    {"FET2575","fet_25_75"},
    {"FEV1","fev_1"},
    {"FEV1_FEV6","fev_1_fev_6"},
    {"FEV1_FVC","fev_1_fvc"},
    {"FEV1_VCext","fev_1_vcext"},
    {"FEV1_VCmax","fev_1_vcmax"},
    {"FEV3","fev_3"},
    {"FEV3_FVC","fev_3_fvc"},
    {"FEV3_VCmax","fev_3_vcmax"},
    {"FEV6","fev_6"},
    {"FEV_25","fev_25"},
    {"FEV_5","fev_5"},
    {"FEV_5_FVC","fev_5_fvc"},
    {"FEV_75","fev_75"},
    {"FEV_75_FEV6","fev_75_fev_6"},
    {"FEV_75_FVC","fev_75_fvc"},
    {"FEV_75_VCmax","fev_75_vcmax"},
    {"FVC","fvc"},
    {"MEF20","mef_20"},
    {"MEF25","mef_25"},
    {"MEF40","mef_40"},
    {"MEF50","mef_50"},
    {"MEF60","mef_60"},
    {"MEF75","mef_75"},
    {"MEF90","mef_90"},
    {"MMEF","mmef"},
    {"MTC1","mtc_1"},
    {"MTC2","mtc_2"},
    {"MTC3","mtc_3"},
    {"MTCR","mtc_r"},
    {"MsgNo","msg_no"},
    {"PEF","pef"},
    {"PEFT","peft"},
    {"PEF_L_Min","pef_l_min"},
    {"T0","t0"},
    {"VCext","vcext"},
    {"VCmax","vcmax"}
};

const q_stringMap SpirometerMeasurement::trialMap =
{
  {"Date","trial_date"},
  {"Number","trial_number"},
  {"Rank","trial_rank"},
  {"RankOriginal","trial_rank_original"},
  {"Accepted","trial_accepted"},
  {"AcceptedOriginal","trial_accepted_original"},
  {"ManualAmbientOverride","trial_manual_ambient_override"}
};

// two measurement types: one is the best values of the trials
// the other is a trial itself.
// best values: result map keys
// trial values: result, trial and channel map keys
//
bool SpirometerMeasurement::isValid() const
{
    bool okResult = true;
    QList<QString> list = resultMap.values();

    // for the FVC test type, these are always NaN even though
    // LL normal and predicted values are provided by default
    //
    list.removeOne("fev_1_vcext");
    list.removeOne("vcext");

    if(typeBestValues == m_type)
    {
        list.removeOne("default_vt_plot_draw_end_index");
    }

    foreach(const auto key, list)
    {
      if(!hasAttribute(key))
      {
        qDebug() << "trial or best values measurement missing results attribute"<<key;
        okResult = false;
        break;
      }
    }
    if(!okResult)
      return okResult;

    bool okChannel = true;
    bool okTrial = true;

    if(m_type == typeTrial)
    {
        // the channels have prefixes flow_ and volume_
        // only the volume channel has DefaultVTPlotDrawEndIdx
        //
      list = channelMap.values();
      QStringList channelKeys;
      foreach(const auto key, list)
      {
        if("default_vt_plot_draw_end_index" == key)
          channelKeys << QString("volume_%1").arg(key);
        else
        {
          channelKeys << QString("flow_%1").arg(key);
          channelKeys << QString("volume_%1").arg(key);
        }
      }

      foreach(const auto key, channelKeys)
      {
        if(!hasAttribute(key))
        {
          qDebug() << "trial measurement missing channel attribute"<<key;
          okChannel = false;
          break;
        }
      }
      if(!okChannel)
        return okChannel;

      list = trialMap.values();
      foreach(const auto key, list)
      {
        if(!hasAttribute(key))
        {
            qDebug() << "trial measurement missing trial attribute"<<key;
          okTrial = false;
          break;
        }
      }
    }
    return okResult && okChannel && okTrial;
}

QStringList SpirometerMeasurement::toStringList(const bool& no_keys) const
{
    QStringList list;
    foreach(const auto x, m_attributes.toStdMap())
    {
      QString key = x.first; // the key
      Measurement::Value value = x.second; // the value
      QString valueStr = value.toString();
      if("flow_values" == key || "volume_values" == key)
      {
          QStringList valueList = valueStr.split(",");
          QStringList shorten;
          // first 3 values truncated at precision of 5
          if(3 < valueList.size())
          {
            for(int i = 0; i < 3; i++)
            {
               double val = valueList.at(i).toDouble();
               shorten.push_back(QString::number(val,'g',5));
            }
            valueStr = QString("%1...").arg(shorten.join(","));
          }
      }
      list << (no_keys ? valueStr : QString("%1: %2").arg(key,valueStr));
    }
    return list;
}

// TODO: generate terse output string
//
QString SpirometerMeasurement::toString() const
{
    QString output = "";
    return output;
}

// set the result type before calling simulate
//
void SpirometerMeasurement::simulate()
{
   reset();
   static int number = 1;
   double mu = QRandomGenerator::global()->generateDouble();
   bool bestValues = typeBestValues == getResultType();

   if(!bestValues)
   {
     setAttribute("trial_date",QDateTime::currentDateTime());
     setAttribute("trial_accepted_original",true);
     setAttribute("trial_accepted",true);
     setAttribute("trial_manual_ambient_override",false);
     setAttribute("trial_number",number);
     setAttribute("trial_rank",number);
     setAttribute("trial_rank_original",number);

     setAttribute("flow_sampling_interval",0.01);
     setAttribute("flow_time_zero_offset",Utilities::interp(-10.0,-5.0,mu));
     // random array of 1000 flow values 18 digits precision
     QStringList flowValues;
     QStringList volumeValues;
     for(int i=0;i<1000;i++)
     {
        double phi = QRandomGenerator::global()->generateDouble();
        double flow = Utilities::interp(0.0000000000000000001,2.0000000000000000001,phi);
        double volume = Utilities::interp(1.0000000000000000001,4.0000000000000000001,phi);
        flowValues << QString::number(flow,'g',18);
        volumeValues << QString::number(volume,'g',18);
     }
     setAttribute("flow_values",flowValues.join(","));
     setAttribute("volume_values",volumeValues.join(","));

     setAttribute("volume_sampling_interval",0.01);
     setAttribute("volume_time_zero_offset",Utilities::interp(-10.0,-5.0,mu));
     setAttribute("volume_default_vt_plot_draw_end_index",990);
     number++;
   }
   if(3 < number)
     number = 1;

   setAttribute("ambient_humidity",50,"%");
   setAttribute("ambient_pressure",50,"hPa");
   setAttribute("ambient_temperature",20,"°C");
   setAttribute("ambient_temperature_fahr",68,"°F");
   setAttribute("bev",Utilities::interp(0.0004,0.336,mu),"L");
   setAttribute("btps_ex",1.02);
   setAttribute("btps_in",Utilities::interp(1.0656,1.1121,mu));
   setAttribute("eotv",Utilities::interp(0.0,1.4241,mu),"L");
   setAttribute("fef_10",Utilities::interp(0.0408,16.2214,mu),"L/s");
   setAttribute("fef_25",Utilities::interp(0.187,14.6166,mu),"L/s");
   setAttribute("fef_25_75",Utilities::interp(0.0607,7.2888,mu),"L/s");
   setAttribute("fef_25_75_predicted",Utilities::interp(0.4912,4.3494,mu),"L/s");
   setAttribute("fef_25_75_ll_normal",Utilities::interp(0.0013,2.7587,mu),"L/s");
   setAttribute("fef_25_75_6",Utilities::interp(0.1136,7.3243,mu),"L/s");
   setAttribute("fef_25_75_fvc",Utilities::interp(0.0706,3.1217,mu));
   setAttribute("fef_40",Utilities::interp(0.0884,11.679,mu),"L/s");
   setAttribute("fef_50",Utilities::interp(0.0612,9.5472,mu),"L/s");
   setAttribute("fef_50_fvc",Utilities::interp(0.0408,3.8559,mu));
   setAttribute("fef_50_vcmax",Utilities::interp(0.0408,3.8559,mu));
   setAttribute("fef_60",Utilities::interp(0.1054,7.0346,mu),"L/s");
   setAttribute("fef_75",Utilities::interp(0.0374,5.134,mu),"L/s");
   setAttribute("fef_75_85",Utilities::interp(0.0422,3.5717,mu),"L/s");
   setAttribute("fef_80",Utilities::interp(0.0068,3.7026,mu),"L/s");
   setAttribute("fet",Utilities::interp(0.6199,22.7556,mu),"s");
   setAttribute("fet_25_75",Utilities::interp(0.1602,7.0799,mu),"s");

   setAttribute("fev_1",Utilities::interp(0.0,205.7,mu),"L");
   setAttribute("fev_1_predicted",Utilities::interp(0.0,205.5,mu),"L");
   setAttribute("fev_1_ll_normal",Utilities::interp(0.4063,4.1306,mu),"L");

   setAttribute("fev_1_fev_6",Utilities::interp(0.2874,1.2262,mu));
   setAttribute("fev_1_fev_6_predicted",Utilities::interp(0.749,0.837,mu));
   setAttribute("fev_1_fev_6_ll_normal",Utilities::interp(0.659,0.75,mu));

   setAttribute("fev_1_fvc",Utilities::interp(0.0,77.3,mu));
   setAttribute("fev_1_fvc_predicted",Utilities::interp(0.0,0.91,mu));
   setAttribute("fev_1_fvc_ll_normal",Utilities::interp(0.597,0.722,mu));

   setAttribute("fev_1_vcext_predicted",Utilities::interp(0.0,1.0,mu));
   setAttribute("fev_1_vcext_ll_normal",Utilities::interp(0.0,1.0,mu));

   setAttribute("fev_1_vcmax",Utilities::interp(0.265,1.0,mu));
   setAttribute("fev_1_vcmax_predicted",Utilities::interp(0.265,1.0,mu));
   setAttribute("fev_1_vcmax_ll_normal",Utilities::interp(0.265,1.0,mu));

   setAttribute("fev_3",Utilities::interp(0.3388,6.8612,mu),"L");
   setAttribute("fev_3_fvc",Utilities::interp(0.4737,1.0,mu));
   setAttribute("fev_3_vcmax",Utilities::interp(0.4737,1.0,mu));

   setAttribute("fev_6",Utilities::interp(0.3221,7.3051,mu),"L");
   setAttribute("fev_6_predicted",Utilities::interp(1.0769,6.4487,mu),"L");
   setAttribute("fev_6_ll_normal",Utilities::interp(0.5806,5.2912,mu),"L");

   setAttribute("fev_25",Utilities::interp(0.2011,3.0394,mu),"L");
   setAttribute("fev_5",Utilities::interp(0.2745,4.434,mu),"L");
   setAttribute("fev_5_fvc",Utilities::interp(0.157,0.9899,mu));

   setAttribute("fev_75",Utilities::interp(0.2988,5.0803,mu),"L");
   setAttribute("fev_75_fev_6",Utilities::interp(0.2291,1.1663,mu));
   setAttribute("fev_75_fvc",Utilities::interp(0.2252,1.0,mu));
   setAttribute("fev_75_vcmax",Utilities::interp(0.2252,1.0,mu));

   setAttribute("fvc",Utilities::interp(0.0,208.4,mu),"L");
   setAttribute("fvc_predicted",Utilities::interp(0.0,6.6706,mu),"L");
   setAttribute("fvc_ll_normal",Utilities::interp(0.6881,5.5081,mu),"L");

   setAttribute("mef_20",Utilities::interp(0.0068,3.7026,mu),"L/s");
   setAttribute("mef_25",Utilities::interp(0.0374,5.134,mu),"L/s");
   setAttribute("mef_40",Utilities::interp(0.1054,7.0346,mu),"L/s");
   setAttribute("mef_50",Utilities::interp(0.0612,9.5472,mu),"L/s");
   setAttribute("mef_60",Utilities::interp(0.0884,11.679,mu),"L/s");
   setAttribute("mef_75",Utilities::interp(0.187,14.6166,mu),"L/s");
   setAttribute("mef_90",Utilities::interp(0.0408,16.2214,mu),"L/s");

   setAttribute("mmef",Utilities::interp(0.0607,7.2888,mu),"L/s");
   setAttribute("mmef_predicted",Utilities::interp(0.4912,4.3494,mu),"L/s");
   setAttribute("mmef_ll_normal",Utilities::interp(0.0013,2.7587,mu),"L/s");

   setAttribute("mtc_1",Utilities::interp(-12.9093,2.8391,mu));
   setAttribute("mtc_2",Utilities::interp(-15.5101,5.1972,mu));
   setAttribute("mtc_3",Utilities::interp(0.5086,22.4604,mu));
   setAttribute("mtc_r",Utilities::interp(-9.6909,0.8822,mu));

   setAttribute("msg_no",0.0);

   setAttribute("pef",Utilities::interp(1.2138,16.6872,mu),"L/s");
   setAttribute("pef_predicted",Utilities::interp(2.6381,11.9719,mu),"L/s");
   setAttribute("pef_ll_normal",Utilities::interp(0.9519,9.0112,mu),"L/s");

   setAttribute("peft",Utilities::interp(-0.3577,1.1764,mu),"s");

   setAttribute("pef_l_min",Utilities::interp(72.828,1001.2319,mu),"L/min");
   setAttribute("pef_l_min_predicted",Utilities::interp(158.2875,718.3125,mu),"L/min");
   setAttribute("pef_l_min_ll_normal",Utilities::interp(57.1125,540.675,mu),"L/min");

   setAttribute("t0",Utilities::interp(0.0017,5.9159,mu),"s");

   setAttribute("vcmax",Utilities::interp(0.3718,7.6995,mu),"L");
   setAttribute("vcmax_predicted",Utilities::interp(0.3718,7.6995,mu),"L");
   setAttribute("vcmax_ll_normal",Utilities::interp(0.3718,7.6995,mu),"L");
}

QDebug operator<<(QDebug dbg, const SpirometerMeasurement& item)
{
    const QString measurementStr = item.toString();
    if(measurementStr.isEmpty())
        dbg.nospace() << "Spirometer Measurement()";
    else
        dbg.nospace() << "Spirometer Measurement(" << measurementStr << " ...)";
    return dbg.maybeSpace();
}
