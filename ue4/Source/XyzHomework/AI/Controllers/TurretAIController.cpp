// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AI/Controllers/TurretAIController.h"

#include "AI/Characters/Turret.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

void ATurretAIController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	ATurret* Turret = CachedTurret.Get();
	if (IsValid(Turret))
	{
		Turret->OnTakeAnyDamage.RemoveDynamic(this, &ATurretAIController::OnPawnDamageTaken);
	}

	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("ATurretAIController::SetPawn(): ATurretAIController can posses only ATurrets."));
		Turret = StaticCast<ATurret*>(InPawn);
		Turret->OnTakeAnyDamage.AddDynamic(this, &ATurretAIController::OnPawnDamageTaken);
		CachedTurret = Turret;
	}
	else
	{
		CachedTurret.Reset();
	}
}

void ATurretAIController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	ATurret* Turret = CachedTurret.Get();
	if (!IsValid(Turret))
	{
		return;
	}

	FAISenseAffiliationFilter AISenseAffiliationFilter = Turret->GetAISenseAffiliationFilter();
	AActor* NewTarget = GetClosestSensedActor(UAISense_Damage::StaticClass(), AISenseAffiliationFilter);
	if (!IsValid(NewTarget))
	{
		NewTarget = GetClosestSensedActor(UAISense_Sight::StaticClass(), AISenseAffiliationFilter);
	}
	Turret->SetCurrentTarget(NewTarget);
}
