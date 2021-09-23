#include "ConnectingState.h"

ConnectingState::ConnectingState(UiHelper* helper, BPMCommunicationHelper* communicationHelper)
    : uiHelper(helper), commHelper(communicationHelper)
{
}

StateEnum ConnectingState::GetState()
{
    return StateEnum::CONNECTING;
}

StateEnum ConnectingState::ProcessMessage(BPMMessage* msg)
{
    switch (msg->GetMsgId()) {
    case (int)ResponseTypeEnum::Ack:
        return OnAck(msg);
        break;
    case (int)ResponseTypeEnum::Buttons:
        LogUnexpectedResponse("Buttons", Buttons::GetPrintableType(msg));
        break;
    case (int)ResponseTypeEnum::BpAverage:
    case (int)ResponseTypeEnum::BpResult:
    case (int)ResponseTypeEnum::DeflatingCuffPressure:
    case (int)ResponseTypeEnum::InflatingCuffPressure:
    case (int)ResponseTypeEnum::Review:
        LogUnexpectedResponse("Data", Data::GetPrintableType(msg));
        break;
    case (int)ResponseTypeEnum::Notification:
        OnNotification(msg);
        break;
    default:
        LogUnexpectedResponse("Unknown", msg->GetFullMsg());
    }

    return GetState();
}

StateEnum ConnectingState::OnAck(BPMMessage* msg)
{
    switch (msg->GetData0()) {
    case (int)AckEnum::HandShake:
        uiHelper->UpdateFirmwareText(QString("%1.%2%3")
            .arg(QString::number(msg->GetData1()), QString::number(msg->GetData2()), QString::number(msg->GetData3())));
        commHelper->Write(BPMCommands::NIBPClear());
        break;
    case (int)AckEnum::Clear:
        commHelper->Write(BPMCommands::NIBPCycle());
        break;
    case (int)AckEnum::Cycle:
        if (msg->GetData1() == 1) {
            uiHelper->UpdateCycleText(1);
            return StateEnum::READY;
        }
        else {
            commHelper->Write(BPMCommands::NIBPCycle());
        }
        break;
    default:
        LogUnexpectedResponse("Ack", Ack::GetPrintableType(msg));
    }

    return GetState();
}

void ConnectingState::OnNotification(BPMMessage* msg)
{
    switch (msg->GetData0()) {
    case (int)NotificationEnum::Reset:
        break;
    default:
        LogUnexpectedResponse("Notification", Notification::GetPrintableType(msg));
        break;
    }
}

void ConnectingState::LogUnexpectedResponse(QString responseType, QString category)
{
    QString logMsg = QString("Unexpected Response (Connecting): %1 (%2)").arg(responseType, category);
    Logger::Log(logMsg);
}