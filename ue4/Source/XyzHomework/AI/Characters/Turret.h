// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "XyzGenericEnums.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionTypes.h"
#include "Turret.generated.h"

UCLASS()
class XYZHOMEWORK_API ATurret : public APawn, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATurret();
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	AActor* GetCurrentTarget() const { return CachedTarget.Get(); }
	void SetCurrentTarget(AActor* NewTarget);
	void SetTurretMode(ETurretMode NewMode);
	float GetTurretMeshHeight() const { return TurretMeshHeight; }
	FAISenseAffiliationFilter GetAISenseAffiliationFilter() const { return AISenseAffiliationFilter; }
	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;

	// IGenericTeamAgentInterface
	virtual FGenericTeamId GetGenericTeamId() const override;
	// ~IGenericTeamAgentInterface

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTurretAttributesComponent* TurretAttributesComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTurretMuzzleComponent* TurretMuzzleComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Turret")
	UStaticMeshComponent* BaseMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Turret")
	UStaticMeshComponent* HeadMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Turret")
	UStaticMeshComponent* BarrelMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret|Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float BaseDiameter = 25.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret|Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float BaseHeight = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret|Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float HeadDiameter = 40.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret|Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float HeadHeight = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret|Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float BarrelDiameter = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret|Geometry", meta = (ClampMin = 1.f, UIMin = 1.f))
	float BarrelLength = 40.f;
	/** Team this actor belongs to. Affects this AI's hostility towards other characters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Turret")
	ETeam Team = ETeam::Enemy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Turret")
	FAISenseAffiliationFilter AISenseAffiliationFilter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret")
	float HeadSearchRotationSpeed = .5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret")
	float HeadTrackRotationSpeed = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret")
	float BarrelPitchRotationSpeed = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret")
	float BarrelMaxPitchAngle = 30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret")
	float BarrelMinPitchAngle = -30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Turret")
	UParticleSystem* ExplosionVFX;

private:
	void SearchEnemy(float DeltaTime) const;
	void TrackEnemy(float DeltaTime);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartFire();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopFire();
	void OnTargetSet();
	void OnDeath();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();

	UPROPERTY()
	USceneComponent* HeadBaseComponent;
	UPROPERTY()
	USceneComponent* BarrelBaseComponent;
	UPROPERTY()
	USceneComponent* BarrelBodyComponent;
	ETurretMode CurrentTurretMode = ETurretMode::Searching;
	UPROPERTY(ReplicatedUsing = OnRep_Target)
	TWeakObjectPtr<AActor> CachedTarget;
	UFUNCTION()
	void OnRep_Target();
	FVector TargetLocation = FVector::ZeroVector;
	float TurretMeshHeight = 0.f;
};
