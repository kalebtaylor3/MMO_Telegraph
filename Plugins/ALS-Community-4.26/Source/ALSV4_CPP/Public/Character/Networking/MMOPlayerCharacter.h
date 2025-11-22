// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ALSCharacter.h"
#include "Character/Networking/MMOSharedTypes.h"
#include "MMOPlayerCharacter.generated.h"


class UWidgetComponent;
class UMMONameplateWidget;
/**
 * 
 */
UCLASS()
class ALSV4_CPP_API AMMOPlayerCharacter : public AALSCharacter
{
	GENERATED_BODY()

public:
	AMMOPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHiddenForLogin(bool bNewHidden);

	// Called from PlayerState when model changes (server + clients)
	void RefreshCharacterModelFromState();

protected:
	// World-space widget over the head
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* NameplateWidgetComponent;

	// Which widget to spawn in that component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMMONameplateWidget> NameplateWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	bool bFaceCamera = true;

	UPROPERTY(ReplicatedUsing = OnRep_IsHiddenForLogin, VisibleAnywhere, Category = "MMO|Login")
	bool bIsHiddenForLogin = true;

	// Shared helper used by BeginPlay + OnRep_PlayerState
	void InitializeNameplate();

	// Apply hiding logic (mesh visibility, collision, movement)
	void ApplyHiddenForLogin();

	UFUNCTION()
	void OnRep_IsHiddenForLogin();

	// === NEW: visual overlay mesh ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMO|Visual")
	USkeletalMeshComponent* CharacterOverlayMesh;

	// Assign these in your BP subclass to actual assets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Visual")
	USkeletalMesh* HumanMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Visual")
	USkeletalMesh* ElfMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Visual")
	USkeletalMesh* OrcMesh;

	// internal helper
	void ApplyCharacterModel(EMMOCharacterModel Model);
	
};
