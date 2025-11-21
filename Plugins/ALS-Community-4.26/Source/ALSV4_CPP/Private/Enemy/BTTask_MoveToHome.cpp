// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_MoveToHome.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_MoveToHome::UBTTask_MoveToHome()
{
	NodeName = "Move To HomeLocation";
}

EBTNodeResult::Type UBTTask_MoveToHome::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!AICon || !BB)
	{
		return EBTNodeResult::Failed;
	}

	const FVector HomeLocation = BB->GetValueAsVector("HomeLocation");

	AICon->MoveToLocation(HomeLocation, 50.f, true, true, true, true);

	return EBTNodeResult::Succeeded;
}
