#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "MMOMobCharacter.h"
#include "BTService_UpdateMobContext.generated.h"

UCLASS()
class ALSV4_CPP_API UBTService_UpdateMobContext : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateMobContext();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
