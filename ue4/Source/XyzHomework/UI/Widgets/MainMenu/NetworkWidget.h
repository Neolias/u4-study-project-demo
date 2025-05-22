// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NetworkWidget.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UNetworkWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNetworkWidgetClosed);

	UPROPERTY(BlueprintAssignable)
	FOnNetworkWidgetClosed OnNetworkWidgetClosed;

protected:
	UFUNCTION(BlueprintPure)
	FText GetNetworkType() const;
	UFUNCTION(BlueprintCallable)
	void ToggleNetworkType();
	UFUNCTION(BlueprintCallable)
	virtual void CloseWidget();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Network Session")
	bool bIsLAN;
};
