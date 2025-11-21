#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_IdleShuffleAroundHome.generated.h"

UCLASS()
class ALSV4_CPP_API UBTTask_IdleShuffleAroundHome : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_IdleShuffleAroundHome();

	/** Home location (vector) – usually "HomeLocation" */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector HomeLocationKey;

	/** Where we want to move for this idle shuffle – e.g. "IdleMoveLocation" */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetLocationKey;

	/** Min distance from home for idle shuffle (small ring instead of on top of home) */
	UPROPERTY(EditAnywhere, Category = "Idle")
	float MinRadius;

	/** Max distance from home for idle shuffle */
	UPROPERTY(EditAnywhere, Category = "Idle")
	float MaxRadius;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
