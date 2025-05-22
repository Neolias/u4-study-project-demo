// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinSessionWidget.h"

#include "XyzGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UJoinSessionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	check(GameInstance->IsA<UXyzGameInstance>());
	CachedGameInstance = StaticCast<UXyzGameInstance*>(GameInstance);
}

void UJoinSessionWidget::FindOnlineSession()
{
	CachedGameInstance->OnMatchFound.AddUFunction(this, FName("OnMatchFound"));
	CachedGameInstance->FindMatch(bIsLAN);
	SearchingSessionState = ESearchingSessionState::Searching;
}

void UJoinSessionWidget::JoinOnlineSession()
{
	CachedGameInstance->JoinOnlineGame();
}

void UJoinSessionWidget::OnMatchFound_Implementation(bool bIsSuccess)
{
	SearchingSessionState = bIsSuccess ? ESearchingSessionState::SessionIsFound : ESearchingSessionState::None;
	CachedGameInstance->OnMatchFound.RemoveAll(this);
}

void UJoinSessionWidget::CloseWidget()
{
	CachedGameInstance->OnMatchFound.RemoveAll(this);
	Super::CloseWidget();
}
