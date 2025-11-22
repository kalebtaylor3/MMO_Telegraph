// Fill out your copyright notice in the Description page of Project Settings.


#include "MMOGameMode.h"

#include "Character/Networking/MMOPlayerState.h"
#include "Character/Networking/MMOPlayerCharacter.h"
#include "Server/MMOAccountStorage.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

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

					// Debug so you can see this fire
					UE_LOG(LogTemp, Log,
						TEXT("AMMOGameMode::Logout - saving location for '%s' -> %s"),
						*Username, *Loc.ToString());

					Storage->UpdateLastLocation(Username, Loc);
				}
			}
		}
	}

	Super::Logout(Exiting);
}