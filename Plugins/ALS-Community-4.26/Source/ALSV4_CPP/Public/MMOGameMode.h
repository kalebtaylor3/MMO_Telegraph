// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MMOGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ALSV4_CPP_API AMMOGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Called on SERVER whenever a player leaves / disconnects
	virtual void Logout(AController* Exiting) override;
	
};
