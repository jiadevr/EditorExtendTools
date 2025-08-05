// Fill out your copyright notice in the Description page of Project Settings.


#include "NotifyTools.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

EAppReturnType::Type UNotifyTools::ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message,
                                                 bool bShowMsgAsWarning)
{
	FText TitleText=bShowMsgAsWarning?FText::FromString(TEXT("Warning")):FText::FromString(TEXT("Message"));
	return FMessageDialog::Open(MsgType,FText::FromString(Message),TitleText);
}

void UNotifyTools::ShowCornerPopupMessage(const FString& Message)
{
	FNotificationInfo NotificationInfo(FText::FromString(Message));
	NotificationInfo.bUseLargeFont=true;
	NotificationInfo.FadeInDuration=10.0f;
	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
}


