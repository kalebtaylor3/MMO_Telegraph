// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Networking/MMOPlayerState.h"
#include "Character/Networking/MMOPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include <Actions/PawnAction.h>

AMMOPlayerState::AMMOPlayerState()
{
	CharacterName = TEXT("Unnamed");
	CharacterLevel = 1;
	Faction = EMMOFaction::None;
	Class = EMMOClass::None;
	CharacterModel = EMMOCharacterModel::Human; // default
	bReplicates = true;
}

void AMMOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMMOPlayerState, CharacterName);
	DOREPLIFETIME(AMMOPlayerState, CharacterLevel);
	DOREPLIFETIME(AMMOPlayerState, Faction);
	DOREPLIFETIME(AMMOPlayerState, Class);
	DOREPLIFETIME(AMMOPlayerState, CharacterModel);
}

void AMMOPlayerState::ApplyCharacterProfile(
	const FString& InName,
	int32                InLevel,
	EMMOFaction          InFaction,
	EMMOClass            InClass,
	EMMOCharacterModel   InModel)
{
	CharacterName = InName;
	CharacterLevel = FMath::Max(1, InLevel);
	Faction = InFaction;
	Class = InClass;
	CharacterModel = InModel;

	// So engine debug panels show it nicely
	SetPlayerName(CharacterName);

	// ?? Now that the profile is valid on the SERVER, reveal the pawn
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController)
	{
		AMMOPlayerCharacter* MMOChar = Cast<AMMOPlayerCharacter>(OwnerController->GetPawn());
		if (MMOChar)
		{
			// No longer in login – unhide overlay/nameplate on all machines
			MMOChar->SetHiddenForLogin(false);

			// Make sure the correct mesh is applied
			MMOChar->RefreshCharacterModelFromState();
		}
	}
}


void AMMOPlayerState::ServerSetCharacterProfile_Implementation(
	const FString& InName,
	int32          InLevel,
	EMMOFaction    InFaction,
	EMMOClass      InClass,
	EMMOCharacterModel InModel)
{
	ApplyCharacterProfile(InName, InLevel, InFaction, InClass, InModel);

	// After profile is confirmed, unhide the pawn for everyone
	if (APawn* Pawn = GetPawn())
	{
		if (AMMOPlayerCharacter* MMOChar = Cast<AMMOPlayerCharacter>(Pawn))
		{
			MMOChar->SetHiddenForLogin(false);
		}
	}
}

void AMMOPlayerState::OnRep_CharacterModel()
{
	if (APawn* Pawn = GetPawn())
	{
		if (AMMOPlayerCharacter* MMOChar = Cast<AMMOPlayerCharacter>(Pawn))
		{
			MMOChar->RefreshCharacterModelFromState();
		}
	}
}

// OnRep callbacks – for now they just exist so you can hook UI later.
// You can add BlueprintImplementableEvents or delegate broadcasts if you want.

void AMMOPlayerState::OnRep_CharacterName()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.0f,
            FColor::Cyan,
            FString::Printf(TEXT("OnRep_CharacterName: %s"), *CharacterName));
    }
}

void AMMOPlayerState::OnRep_CharacterLevel()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.0f,
            FColor::Yellow,
            FString::Printf(TEXT("OnRep_CharacterLevel: %d"), CharacterLevel));
    }
}

void AMMOPlayerState::OnRep_Faction()
{
	if (GEngine)
	{
		const UEnum* Enum = StaticEnum<EMMOFaction>();
		const FString FactionName = Enum
			? Enum->GetDisplayNameTextByValue((int64)Faction).ToString()
			: TEXT("Unknown");

		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Green,
			FString::Printf(TEXT("OnRep_Faction: %s"), *FactionName, (int32)Faction));
	}
}

void AMMOPlayerState::OnRep_Class()
{
	if (GEngine)
	{
		const UEnum* Enum = StaticEnum<EMMOClass>();
		const FString ClassName = Enum
			? Enum->GetDisplayNameTextByValue((int64)Class).ToString()
			: TEXT("Unknown");

		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Magenta,
			FString::Printf(TEXT("OnRep_Class: %s"), *ClassName, (int32)Class));
	}
}
