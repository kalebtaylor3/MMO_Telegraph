// Fill out your copyright notice in the Description page of Project Settings.


#include "MMOGameMode.h"

#include "Character/Networking/MMOPlayerState.h"
#include "Character/Networking/MMOPlayerCharacter.h"
#include "Server/MMOAccountStorage.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void AMMOGameMode::BeginPlay()
{
	Super::BeginPlay();

	// GameMode only exists on the server, but HasAuthority() keeps intent clear.
	if (HasAuthority())
	{
		// Autosave every 10 seconds (tune this as you like)
		GetWorldTimerManager().SetTimer(
			AutosaveTimerHandle,
			this,
			&AMMOGameMode::AutosaveAllPlayers,
			10.0f,
			true);

		UE_LOG(LogTemp, Log, TEXT("AMMOGameMode::BeginPlay - started autosave timer."));
	}
}

void AMMOGameMode::Logout(AController* Exiting)
{
	// Exiting is only valid on the server
	APlayerController* PC = Cast<APlayerController>(Exiting);
	if (PC)
	{
		AMMOPlayerState* PS = PC->GetPlayerState<AMMOPlayerState>();
		AMMOPlayerCharacter* MMOChar = Cast<AMMOPlayerCharacter>(PC->GetPawn());

		if (PS && MMOChar)
		{
			const FString& Username = PS->GetAccountUsername();

			if (!Username.IsEmpty())
			{
				UGameInstance* GI = GetGameInstance();
				UMMOAccountStorage* Storage = GI ? GI->GetSubsystem<UMMOAccountStorage>() : nullptr;

				if (Storage)
				{
					const FVector Loc = MMOChar->GetActorLocation();

					Storage->UpdateLastLocation(Username, Loc);
				}
			}
		}
	}

	Super::Logout(Exiting);
}

void AMMOGameMode::AutosaveAllPlayers()
{
	UGameInstance* GI = GetGameInstance();
	UMMOAccountStorage* Storage = GI ? GI->GetSubsystem<UMMOAccountStorage>() : nullptr;
	if (!Storage)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		AMMOPlayerState* PS = PC->GetPlayerState<AMMOPlayerState>();
		if (!PS)
		{
			continue;
		}

		const FString& Username = PS->GetAccountUsername();
		if (Username.IsEmpty())
		{
			continue; // no bound account yet
		}

		APawn* Pawn = PC->GetPawn();
		if (!Pawn)
		{
			continue;
		}

		const FVector Loc = Pawn->GetActorLocation();

		// This uses your existing UpdateLastLocation function:
		if (!Storage->UpdateLastLocation(Username, Loc))
		{
			UE_LOG(LogTemp, Verbose,
				TEXT("AutosaveAllPlayers: failed to update location for '%s'"),
				*Username);
		}
	}
}