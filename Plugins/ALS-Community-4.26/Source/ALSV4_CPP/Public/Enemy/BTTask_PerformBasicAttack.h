// BTTask_PerformBasicAttack.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PerformBasicAttack.generated.h"

UCLASS()
class ALSV4_CPP_API UBTTask_PerformBasicAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PerformBasicAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
