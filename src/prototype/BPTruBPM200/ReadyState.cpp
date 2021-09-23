#include "ReadyState.h"

ReadyState::ReadyState(UiHelper* helper, BPMCommunicationHelper* communicationHelper)
    : uiHelper(helper), commHelper(communicationHelper)
{
}

StateEnum ReadyState::GetState()
{
    return StateEnum::READY;
}

StateEnum ReadyState::ProcessMessage(BPMMessage* msg)
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
        LogUnexpectedResponse("Data", Data::GetPrintableType(msg));
        commHelper->Write(BPMCommands::NIBPStop());
        break;
    case (int)ResponseTypeEnum::Notification:
        return OnNotification(msg);
    default:
        LogUnexpectedResponse("Unknown", msg->GetFullMsg());
    }

    return GetState();
}

StateEnum ReadyState::OnAck(BPMMessage* msg)
{
    switch (msg->GetData0()) {
    case (int)AckEnum::Clear:
        uiHelper->ClearResults();
        commHelper->Write(BPMCommands::NIBPStart());
        break;
    case (int)AckEnum::Cycle:
        // Cycle until manual
        if (msg->GetData1() == 0) {
            uiHelper->UpdateCycleText(0);
            commHelper->Write(BPMCommands::NIBPStart());
        }
        else {
            commHelper->Write(BPMCommands::NIBPCycle());
        }
        break;
    case (int)AckEnum::Start:
        uiHelper->UpdateCycleText(msg->GetData1());
        uiHelper->UpdateReadingText(msg->GetData2());
        return StateEnum::MEASURING;
    default:
        LogUnexpectedResponse("Button", Ack::GetPrintableType(msg));
    }

    return GetState();
}

StateEnum ReadyState::OnNotification(BPMMessage* msg)
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

StateEnum ReadyState::OnButton(BPMMessage* msg)
{
    switch (msg->GetData0()) {
    case (int)ButtonsEnum::Cleared:
        uiHelper->ClearResults();
        break;
    case (int)ButtonsEnum::Cycled:
        uiHelper->UpdateCycleText(msg->GetData1());
        break;
    case (int)ButtonsEnum::Started:
        uiHelper->UpdateCycleText(msg->GetData1());
        return StateEnum::MEASURING;
    default:
        LogUnexpectedResponse("Button", Buttons::GetPrintableType(msg));
        break;
    }
    return GetState();
}

void ReadyState::LogUnexpectedResponse(QString responseType, QString category)
{
    QString logMsg = QString("Unexpected Response (Ready): % 1 (% 2)").arg(responseType, category);
    Logger::Log(logMsg);
}
