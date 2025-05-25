// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Animation/AnimInstance.h"
#include "XyzBaseCharacterAnimInstance.generated.h"

UCLASS()
class XYZHOMEWORK_API UXyzBaseCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsFalling = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsCrouching = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsSprinting = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsOutOfStamina = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsProne = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsSwimming = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsOnLadder = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsOnZipline = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsWallRunning = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	float LadderSpeedRatio = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsAiming = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsReloading = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	bool bIsPrimaryItemEquipped = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	FVector Velocity = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States", meta = (UIMin = -180.f, UIMax = 180.f))
	float MovementDirection = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States", meta = (UIMin = 0.f))
	float MovementSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	EWallRunSide CurrentWallRunSide = EWallRunSide::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States")
	EEquipmentItemType CurrentEquipmentItemType = EEquipmentItemType::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation|States", meta = (UIMin = -180.f, UIMax = 180.f))
	float CharacterPitch = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation", meta = (UIMin = -180.f, UIMax = 180.f))
	float CameraPitch = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation", meta = (UIMin = -180.f, UIMax = 180.f))
	FRotator AimRotation = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation")
	FVector LeftFootEffectorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation")
	FVector RightFootEffectorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation")
	FVector PelvisEffectorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Animation")
	FTransform CurrentRangedWeaponForeGripTransform;

private:
	float CalculateCameraPitch() const;

	TWeakObjectPtr<class AXyzBaseCharacter> CachedBaseCharacter;
};
