// BTTask_MoveToHome.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToHome.generated.h"

UCLASS()
class ALSV4_CPP_API UBTTask_MoveToHome : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToHome();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
