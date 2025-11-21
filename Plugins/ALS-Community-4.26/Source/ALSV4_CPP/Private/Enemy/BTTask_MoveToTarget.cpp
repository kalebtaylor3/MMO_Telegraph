// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_MoveToTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UBTTask_MoveToTarget::UBTTask_MoveToTarget()
{
	NodeName = "Move To TargetActor";
}

EBTNodeResult::Type UBTTask_MoveToTarget::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!AICon || !BB)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject("TargetActor"));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	const float AttackRange = BB->GetValueAsFloat("AttackRange");

	AICon->MoveToActor(Target, AttackRange * 0.7f, true, true, true, 0, true);

	return EBTNodeResult::Succeeded; // we let AIController handle continuation
}
