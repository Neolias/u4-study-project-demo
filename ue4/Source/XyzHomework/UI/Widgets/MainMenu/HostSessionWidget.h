// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkWidget.h"
#include "HostSessionWidget.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UHostSessionWidget : public UNetworkWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	void CreateSession();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Session")
	FName ServerName;
};
