// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Actors/Environment/EnvironmentActor.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CapsuleComponent.h"

void AEnvironmentActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(InteractiveVolume))
	{
		InteractiveVolume->OnComponentBeginOverlap.AddDynamic(this, &AEnvironmentActor::OnInteractiveVolumeBeginOverlap);
		InteractiveVolume->OnComponentEndOverlap.AddDynamic(this, &AEnvironmentActor::OnInteractiveVolumeEndOverlap);
	}
}

void AEnvironmentActor::OnInteractiveVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(OtherActor);
	if (!IsValid(BaseCharacter) || !OverlapsBaseCharacterCapsuleComponent(BaseCharacter, OtherComp))
	{
		return;
	}
	BaseCharacter->RegisterEnvironmentActor(this);
}

void AEnvironmentActor::OnInteractiveVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(OtherActor);
	if (!IsValid(BaseCharacter) || !OverlapsBaseCharacterCapsuleComponent(BaseCharacter, OtherComp))
	{
		return;
	}
	BaseCharacter->UnRegisterEnvironmentActor(this);
}

bool AEnvironmentActor::OverlapsBaseCharacterCapsuleComponent(const AXyzBaseCharacter* BaseCharacter, const UPrimitiveComponent* Component) const
{
	if (Component != BaseCharacter->GetCapsuleComponent())
	{
		return false;
	}
	return true;
}
