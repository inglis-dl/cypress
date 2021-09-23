#include "MeasuringState.h"

MeasuringState::MeasuringState(UiHelper* helper, BPMCommunicationHelper* communicationHelper)
    : uiHelper(helper), commHelper(communicationHelper)
{
}

StateEnum MeasuringState::GetState()
{
    return StateEnum::MEASURING;
}

StateEnum MeasuringState::ProcessMessage(BPMMessage* msg)
{
    switch (msg->GetMsgId()) {
    case (int)ResponseTypeEnum::Ack:
        return OnAck(msg);
        break;
    case (int)ResponseTypeEnum::Buttons:
        return OnButton(msg);
    case (int)ResponseTypeEnum::BpAverage:
    case (int)ResponseTypeEnum::BpResult:
    case (int)ResponseTypeEnum::DeflatingCuffPressure:
    case (int)ResponseTypeEnum::InflatingCuffPressure:
    case (int)ResponseTypeEnum::Review:
        OnData(msg);
        break;
    case (int)ResponseTypeEnum::Notification:
        return OnNotification(msg);
    default:
        LogUnexpectedResponse("Unknown", msg->GetFullMsg());
    }

    return GetState();
}

StateEnum MeasuringState::OnAck(BPMMessage* msg)
{
    switch (msg->GetData0()) {
    case (int)AckEnum::Clear:
        return StateEnum::READY;
    case (int)AckEnum::Stop:
        EndMeasures();
        break;
    default:
        LogUnexpectedResponse("Ack", Ack::GetPrintableType(msg));
    }

    return GetState();
}

StateEnum MeasuringState::OnNotification(BPMMessage* msg)
{
    switch (msg->GetData0()) {
    case (int)NotificationEnum::Reset:
        return StateEnum::CONNECTING;
        break;
    default:
        LogUnexpectedResponse("Notification", Notification::GetPrintableType(msg));
        break;
    }
    return GetState();
}

StateEnum MeasuringState::OnButton(BPMMessage* msg)
{
    switch (msg->GetData0()) {
    case (int)ButtonsEnum::Stopped:
        EndMeasures();
        break;
    default:
        LogUnexpectedResponse("Button", Buttons::GetPrintableType(msg));
        break;
    }
    return GetState();
}

StateEnum MeasuringState::OnData(BPMMessage* msg)
{
    switch (msg->GetMsgId()) {
    case (int)DataEnum::InflatingCuffPressure:
        if (startTime == 0) {
            startTime = QDateTime::currentMSecsSinceEpoch();
        }
        uiHelper->UpdateCuffPressure(msg->GetData0() + msg->GetData1(), true);
        break;
    case (int)DataEnum::DeflatingCuffPressure:
        uiHelper->UpdateCuffPressure(msg->GetData0() + msg->GetData1(), false);
        break;
    case (int)DataEnum::BPResult:
        EndReading(msg->GetData1(), msg->GetData2(), msg->GetData3());
        if (uiHelper->GetCycleVal() == 0) {
            EndMeasures();
        }
        break;
    case (int)DataEnum::BPAverage:
        break;
    case (int)DataEnum::Review:
        break;
    default:
        LogUnexpectedResponse("Data", Data::GetPrintableType(msg));
        break;
    }
    return GetState();
}

void MeasuringState::EndReading(int systolic, int diastolic, int pulse)
{
    long endTime = QDateTime::currentMSecsSinceEpoch();
    uiHelper->AddResult(startTime, endTime, systolic, diastolic, pulse);
    uiHelper->IncrementReading();
    startTime = 0;
}

void MeasuringState::EndMeasures()
{
    uiHelper->UpdateCuffPressure(0);
    commHelper->Write(BPMCommands::NIBPClear());
}

void MeasuringState::LogUnexpectedResponse(QString responseType, QString category)
{
    QString logMsg = QString("Unexpected Response (Measuring): %1 (%2)").arg(responseType, category);
    Logger::Log(logMsg);
}
