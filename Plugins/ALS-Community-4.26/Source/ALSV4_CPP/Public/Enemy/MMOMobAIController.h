#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Enemy/MMOMobCharacter.h"        // adjust path if needed
#include "MMOMobAIController.generated.h"

class UBehaviorTree;
class UBlackboardComponent;

UCLASS()
class ALSV4_CPP_API AMMOMobAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMMOMobAIController();

	virtual void OnPossess(APawn* InPawn) override;

	/** Called by mob character when something enters aggro */
	void NotifyAggro(APawn* NewTarget);

	/** Convenience getter */
	FORCEINLINE UBlackboardComponent* GetBB() const { return BlackboardComp; }

protected:
	/** Behavior tree asset to run for this mob */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

	/** Blackboard component driving this AI */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBlackboardComponent* BlackboardComp;
};
