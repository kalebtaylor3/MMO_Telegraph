// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/MMOMobAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

AMMOMobAIController::AMMOMobAIController()
{
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void AMMOMobAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMMOMobCharacter* Mob = Cast<AMMOMobCharacter>(InPawn);
	if (!Mob || !BehaviorTreeAsset)
	{
		return;
	}

	if (UBlackboardData* BBAsset = BehaviorTreeAsset->BlackboardAsset)
	{
		BlackboardComp->InitializeBlackboard(*BBAsset);

		// Use the pawn's world location at possess-time as "home"
		const FVector HomeLocation = Mob->GetActorLocation();
		BlackboardComp->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);

		BlackboardComp->SetValueAsFloat(TEXT("AttackRange"), Mob->AttackRange);
		BlackboardComp->SetValueAsFloat(TEXT("LeashDistance"), Mob->LeashDistance);
		BlackboardComp->SetValueAsBool(TEXT("HasTarget"), false);
		BlackboardComp->SetValueAsBool(TEXT("IsInAttackRange"), false);
		BlackboardComp->SetValueAsBool(TEXT("IsOutOfLeash"), false);
		BlackboardComp->SetValueAsObject(TEXT("TargetActor"), nullptr);
		BlackboardComp->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(EMMOMobAIState::Idle));

		Mob->SetAIState(EMMOMobAIState::Idle);

		RunBehaviorTree(BehaviorTreeAsset);
	}
}

void AMMOMobAIController::NotifyAggro(APawn* NewTarget)
{
	if (!BlackboardComp || !NewTarget)
	{
		return;
	}

	BlackboardComp->SetValueAsObject(TEXT("TargetActor"), NewTarget);
	BlackboardComp->SetValueAsBool(TEXT("HasTarget"), true);
	BlackboardComp->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(EMMOMobAIState::Combat));

	if (AMMOMobCharacter* Mob = Cast<AMMOMobCharacter>(GetPawn()))
	{
		Mob->SetAIState(EMMOMobAIState::Combat);
	}
}
