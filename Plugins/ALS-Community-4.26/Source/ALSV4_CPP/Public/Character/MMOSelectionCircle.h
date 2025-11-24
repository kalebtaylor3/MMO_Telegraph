// MMOSelectionCircle.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MMOSelectionCircle.generated.h"

class UDecalComponent;

UCLASS()
class ALSV4_CPP_API AMMOSelectionCircle : public AActor
{
	GENERATED_BODY()

public:
	AMMOSelectionCircle();

	virtual void BeginPlay() override;

	// Show / hide without destroying
	void SetActive(bool bInActive);

	// Set color to match selection (ally/enemy/NPC)
	void SetColor(const FLinearColor& InColor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
	UDecalComponent* DecalComp;

private:
	UPROPERTY()
	UMaterialInstanceDynamic* DynMaterial;
};
