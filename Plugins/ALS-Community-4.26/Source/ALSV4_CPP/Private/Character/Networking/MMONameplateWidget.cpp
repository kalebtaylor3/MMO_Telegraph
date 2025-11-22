// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Networking/MMONameplateWidget.h"

#include "Character/Networking/MMOPlayerState.h"

void UMMONameplateWidget::InitializeFromPlayerState(AMMOPlayerState* InPlayerState)
{
	ObservedPlayerState = InPlayerState;
}

FText UMMONameplateWidget::GetNameplateText() const
{
	if (!ObservedPlayerState)
	{
		return FText::GetEmpty();
	}

	// Build faction string from enum
	FString FactionString;
	switch (ObservedPlayerState->Faction)
	{
	case EMMOFaction::Alliance:
		FactionString = TEXT("Alliance");
		break;
	case EMMOFaction::Horde:
		FactionString = TEXT("Horde");
		break;
	case EMMOFaction::Neutral:
	default:
		FactionString = TEXT("Neutral");
		break;
	}

	const FString Result = FString::Printf(
		TEXT("(%s)%s"),
		*FactionString,
		*ObservedPlayerState->CharacterName);

	return FText::FromString(Result);
}

FText UMMONameplateWidget::GetFactionText() const
{
	if (!ObservedPlayerState)
	{
		return FText::GetEmpty();
	}

	switch (ObservedPlayerState->Faction)
	{
	case EMMOFaction::Alliance: return FText::FromString(TEXT("(Alliance)"));
	case EMMOFaction::Horde:    return FText::FromString(TEXT("(Horde)"));
	default:                    return FText::FromString(TEXT("(Neutral)"));
	}
}

FText UMMONameplateWidget::GetPlayerNameText() const
{
	if (!ObservedPlayerState)
	{
		return FText::GetEmpty();
	}

	return FText::FromString(ObservedPlayerState->CharacterName);
}