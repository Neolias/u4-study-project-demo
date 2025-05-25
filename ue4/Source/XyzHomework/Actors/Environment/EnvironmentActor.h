// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Characters/XyzBaseCharacter.h"
#include "GameFramework/Actor.h"
#include "EnvironmentActor.generated.h"

/** Base class of environment actors, such as ladders and ziplines. */
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
