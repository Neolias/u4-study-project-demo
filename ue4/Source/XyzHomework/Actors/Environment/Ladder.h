// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Actors/Environment/EnvironmentActor.h"
#include "Ladder.generated.h"

UCLASS(Blueprintable)
class XYZHOMEWORK_API ALadder : public AEnvironmentActor
{
	GENERATED_BODY()

public:
	ALadder();
	float GetLadderHeight() const { return LadderHeight; }
	UAnimMontage* GetAttachFromTopAnimMontage() const { return AttachFromTopAnimMontage; }
	UAnimMontage* GetAttachFromTopFPAnimMontage() const { return AttachFromTopFPAnimMontage; }
	bool IsOnTop() const { return bIsOnTop; }
	FVector GetAttachFromTopAnimMontageStartLocation() const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void OnInteractiveVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnInteractiveVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ladder Parameters")
	UStaticMeshComponent* LeftRailMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ladder Parameters")
	UStaticMeshComponent* RightRailMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ladder Parameters")
	UInstancedStaticMeshComponent* StepsMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ladder Parameters")
	UPrimitiveComponent* TopInteractiveVolume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float LadderWidth = 70.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float LadderHeight = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float TopInteractiveVolumeDepth = 70.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float TopInteractiveVolumeHeight = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BottomInteractiveVolumeDepth = 70.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StepHeight = 40.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters")
	float FirstStepOffset = 40.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters")
	float LastStepOffset = 40.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters")
	UAnimMontage* AttachFromTopAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters")
	UAnimMontage* AttachFromTopFPAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder Parameters")
	FVector AttachFromTopAnimMontageStartOffset = FVector::ZeroVector;

	float RailRadiusScale = .75;
	bool bIsOnTop = false;
};
