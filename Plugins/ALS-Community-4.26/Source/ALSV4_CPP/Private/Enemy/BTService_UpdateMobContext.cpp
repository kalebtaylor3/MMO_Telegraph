// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTService_UpdateMobContext.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_UpdateMobContext::UBTService_UpdateMobContext()
{
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = false;
	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UBTService_UpdateMobContext::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float                   DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	AMMOMobCharacter* Mob = Cast<AMMOMobCharacter>(Pawn);

	if (!BB || !Mob)
	{
		return;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject("TargetActor"));
	const FVector HomeLocation = BB->GetValueAsVector("HomeLocation");
	const float   AttackRange = BB->GetValueAsFloat("AttackRange");
	const float   LeashDistance = BB->GetValueAsFloat("LeashDistance");

	// --- Distances ---
	const float DistFromHome = FVector::Dist2D(Mob->GetActorLocation(), HomeLocation);
	const float ReturnTolerance = 150.f;   // how close is "back home" (tweak)

	bool bHasTarget = Target != nullptr;
	bool bIsInAttackRange = false;

	// Are we truly out of leash?
	bool bIsOutOfLeash = DistFromHome > LeashDistance;

	if (Target)
	{
		const float DistToTarget = FVector::Dist2D(Mob->GetActorLocation(), Target->GetActorLocation());
		bIsInAttackRange = DistToTarget <= AttackRange;

		// If we broke leash while chasing, drop target
		if (bIsOutOfLeash)
		{
			BB->ClearValue("TargetActor");
			bHasTarget = false;
			Target = nullptr;
		}
	}

	// --- Write BB booleans ---
	BB->SetValueAsBool("HasTarget", bHasTarget);
	BB->SetValueAsBool("IsInAttackRange", bIsInAttackRange);
	BB->SetValueAsBool("IsOutOfLeash", bIsOutOfLeash);

	// --- Decide AIState with a home-tolerance so they don't shuffle ---
	EMMOMobAIState CurrentState = Mob->AIState;
	EMMOMobAIState NewState = CurrentState;

	if (bHasTarget && !bIsOutOfLeash)
	{
		// Normal combat
		NewState = EMMOMobAIState::Combat;
	}
	else if (bIsOutOfLeash)
	{
		// Too far from home -> go back
		NewState = EMMOMobAIState::Returning;
	}
	else
	{
		// Not out of leash and no target
		if (CurrentState == EMMOMobAIState::Returning)
		{
			// Stay Returning until we're comfortably near home, then Idle
			const bool bBackAtHome = DistFromHome <= ReturnTolerance;
			NewState = bBackAtHome ? EMMOMobAIState::Idle : EMMOMobAIState::Returning;
		}
		else
		{
			// Default chill state
			NewState = EMMOMobAIState::Idle;
		}
	}

	if (NewState != CurrentState)
	{
		Mob->SetAIState(NewState);
		BB->SetValueAsEnum("AIState", static_cast<uint8>(NewState));
	}
}

