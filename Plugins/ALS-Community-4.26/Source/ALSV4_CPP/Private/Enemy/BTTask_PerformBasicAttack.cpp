// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_PerformBasicAttack.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Enemy/MMOMobCharacter.h"

UBTTask_PerformBasicAttack::UBTTask_PerformBasicAttack()
{
	NodeName = "Perform Basic Attack";
}

EBTNodeResult::Type UBTTask_PerformBasicAttack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	AMMOMobCharacter* Mob = Cast<AMMOMobCharacter>(AICon->GetPawn());
	if (!Mob)
	{
		return EBTNodeResult::Failed;
	}

	Mob->PerformBasicAttack();

	return EBTNodeResult::Succeeded;
}

