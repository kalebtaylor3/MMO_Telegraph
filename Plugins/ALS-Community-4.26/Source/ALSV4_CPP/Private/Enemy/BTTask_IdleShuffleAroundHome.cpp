// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_IdleShuffleAroundHome.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_IdleShuffleAroundHome::UBTTask_IdleShuffleAroundHome()
{
	NodeName = TEXT("Pick Idle Shuffle Location");

	MinRadius = 100.f;
	MaxRadius = 250.f;

	// We only do a one-shot computation, so we don't need tick
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_IdleShuffleAroundHome::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	// Get home location from BB (if not set, fall back to current actor location)
	FVector HomeLocation = BB->GetValueAsVector(HomeLocationKey.SelectedKeyName);
	if (!HomeLocationKey.SelectedKeyName.IsNone() && HomeLocation.IsNearlyZero())
	{
		// If the key exists but is zero (and you want to be safe), fall back
		HomeLocation = Pawn->GetActorLocation();
	}
	else if (HomeLocationKey.SelectedKeyType == nullptr || HomeLocationKey.SelectedKeyName.IsNone())
	{
		HomeLocation = Pawn->GetActorLocation();
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	// We want a random point in a ring [MinRadius, MaxRadius] around HomeLocation
	const float UseMinRadius = FMath::Clamp(MinRadius, 0.f, MaxRadius);

	// Pick a random direction on the X/Y plane
	const float AngleRad = FMath::FRandRange(0.f, 2.f * PI);
	const float Radius = FMath::FRandRange(UseMinRadius, MaxRadius);

	const FVector Offset(
		FMath::Cos(AngleRad) * Radius,
		FMath::Sin(AngleRad) * Radius,
		0.f);

	const FVector DesiredLocation = HomeLocation + Offset;

	FNavLocation NavResult;
	// Project our desired point onto the navmesh
	const bool bFound = NavSys->ProjectPointToNavigation(
		DesiredLocation,
		NavResult,
		FVector(200.f, 200.f, 200.f)); // search extents

	if (!bFound)
	{
		// Fallback: just use home if we can't find a valid point
		BB->SetValueAsVector(TargetLocationKey.SelectedKeyName, HomeLocation);
		return EBTNodeResult::Succeeded;
	}

	// Write the result into the BB
	BB->SetValueAsVector(TargetLocationKey.SelectedKeyName, NavResult.Location);

	return EBTNodeResult::Succeeded;
}
