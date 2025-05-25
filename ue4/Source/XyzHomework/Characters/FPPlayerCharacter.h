// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Characters/PlayerCharacter.h"
#include "FPPlayerCharacter.generated.h"

/** DEPRECATED. First-person character class that mainly manages the first-person camera. */
UCLASS()
class XYZHOMEWORK_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	explicit AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual FRotator GetViewRotation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player")
	USkeletalMeshComponent* FPMeshComponent;

private:
	bool IsFPMontagePlaying() const;
	
#pragma region CAMERA

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* FPCameraComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera")
	FName CameraSocket = FName("CameraSocket");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera", meta = (ClampMin = 0.f, UIMin = 0.f))
	float CameraAlignmentBlendSpeed = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera", meta = (UIMin = -89.9f, UIMax = 89.9f))
	float LadderCameraMinPitch = -60.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera", meta = (UIMin = -89.9f, UIMax = 89.9f))
	float LadderCameraMaxPitch = 80.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera", meta = (UIMin = -359.9f, UIMax = 0.f))
	float LadderCameraMinYaw = -60.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera", meta = (UIMin = 0.f, UIMax = 359.9f))
	float LadderCameraMaxYaw = 60.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera", meta = (UIMin = -89.9f, UIMax = 89.9f))
	float ZiplineCameraMinPitch = -89.9f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera", meta = (UIMin = -89.9f, UIMax = 89.9f))
	float ZiplineCameraMaxPitch = 89.9f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera", meta = (UIMin = -359.9f, UIMax = 0.f))
	float ZiplineCameraMinYaw = -90.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera", meta = (UIMin = 0.f, UIMax = 359.9f))
	float ZiplineCameraMaxYaw = 90.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player|Camera")
	UCurveFloat* WallRunCameraTiltCurve;

private:
	bool IsAligningFPCameraToSocketRotation() const;
	void StartFPCameraAlignment();
	void EndFPCameraAlignment();
	void UpdateCameraAlignment(float DeltaSeconds);
	void UpdateWallRunCameraTilt(float Value) const;
	void StartWallRunCameraTilt();
	void EndWallRunCameraTilt();

	/** Forced camera rotation that ignores the camera socket rotation during animations. */
	FRotator ForcedTargetControlRotation = FRotator::ZeroRotator;
	FTimeline WallRunCameraTiltTimeline;
	bool bIsForcedToAlignFPCamera = false;
	float AnimMontageCameraBlendSpeed = 50.f;
#pragma endregion

#pragma region MOVEMENT

public:
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStopProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

protected:
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, const FMantlingMovementParameters& MantlingParameters) override;
	virtual void StartHardLand() override;
	virtual void StopHardLand() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|FP Player")
	UAnimMontage* SlideFPAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|FP Player")
	UAnimMontage* HardLandFPAnimMontage;

private:
	void OnStopMantle();

	bool bWantsToEndMantle = false;
	bool bWantsToEndHardLand = false;
	FTimerHandle FPMantlingTimerHandle;
	FTimerHandle FPSlideTimerHandle;
#pragma endregion

#pragma region ENVIRONMENT ACTORS

protected:
	virtual void OnAttachedToLadderFromTop(ALadder* Ladder) override;

private:
	void OnAttachedToLadder();
	void OnDetachedFromLadder();
	void OnAttachedToZipline();
	void OnDetachedFromZipline();

	bool bWantsToAttachToLadder = false;
	FTimerHandle FPLadderTimerHandle;
#pragma endregion

#pragma region ATTRIBUTES

protected:
	virtual void OnDeath(bool bShouldPlayAnimMontage) override;
	virtual void StartOutOfStaminaInternal() override;
	virtual void StopOutOfStaminaInternal() override;

private:
	bool bWantsToEndOutOfStamina = false;

#pragma endregion
};
