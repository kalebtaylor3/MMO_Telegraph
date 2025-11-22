// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MMONameplateWidget.generated.h"


class AMMOPlayerState;

/**
 * 
 */
UCLASS()
class ALSV4_CPP_API UMMONameplateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// PlayerState we are showing name/faction for
	UPROPERTY(BlueprintReadOnly, Category = "Nameplate")
	AMMOPlayerState* ObservedPlayerState = nullptr;

	// Called from character to bind the correct PS
	UFUNCTION(BlueprintCallable, Category = "Nameplate")
	void InitializeFromPlayerState(AMMOPlayerState* InPlayerState);

	// Blueprint can bind TextBlock.Text to this
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Nameplate")
	FText GetNameplateText() const;

	// NEW: split faction + name
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Nameplate")
	FText GetFactionText() const;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Nameplate")
	FText GetPlayerNameText() const;
	
};
