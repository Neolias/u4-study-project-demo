// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/XyzBaseCharacter.h"
#include "GameFramework/Actor.h"
#include "EnvironmentActor.generated.h"

/**
 *
 */
UCLASS(Abstract, NotBlueprintable)
class XYZHOMEWORK_API AEnvironmentActor : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnInteractiveVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnInteractiveVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	bool OverlapsBaseCharacterCapsuleComponent(const AXyzBaseCharacter* BaseCharacter, const UPrimitiveComponent* Component) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Actor")
	UPrimitiveComponent* InteractiveVolume;
};
