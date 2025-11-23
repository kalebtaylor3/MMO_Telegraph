#pragma once

#include "CoreMinimal.h"
#include "Character/ALSCharacter.h"
#include "MMOMobCharacter.generated.h"

class USphereComponent;

UENUM(BlueprintType)
enum class EMMOMobAIState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Combat      UMETA(DisplayName = "Combat"),
	Returning   UMETA(DisplayName = "Returning")
};

UCLASS()
class ALSV4_CPP_API AMMOMobCharacter : public AALSCharacter
{
	GENERATED_BODY()

public:
	AMMOMobCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	/** Aggro radius for detecting players */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMO|Mob")
	float AggroRadius = 900.f;

	/** How far from spawn we allow chasing before leashing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMO|Mob")
	float LeashDistance = 2000.f;

	/** Attack range for melee */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMO|Mob")
	float AttackRange = 250.f;

	/** Time between basic attacks */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMO|Mob")
	float AttackInterval = 2.0f;

	/** Saved at BeginPlay; used as home/return point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMO|Mob")
	FVector SpawnLocation;

	/** Current AI state (for debugging + BT keys) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMO|Mob")
	EMMOMobAIState AIState = EMMOMobAIState::Idle;

	/** Aggro overlap for simple detection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMO|Mob")
	USphereComponent* AggroSphere;

	/** Called from BTTask_PerformBasicAttack */
	UFUNCTION(BlueprintCallable, Category = "MMO|Mob")
	void PerformBasicAttack();

	/** Helper for BT / controller */
	void SetAIState(EMMOMobAIState NewState);

	USkeletalMeshComponent* GetHighlightMesh() const { return CharacterOverlayMesh; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMO|Visual")
	USkeletalMeshComponent* CharacterOverlayMesh;

protected:
	UFUNCTION()
	void OnAggroSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32                OtherBodyIndex,
		bool                 bFromSweep,
		const FHitResult& SweepResult);
};
