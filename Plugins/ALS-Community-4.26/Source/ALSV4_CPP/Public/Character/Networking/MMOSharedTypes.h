#pragma once

#include "CoreMinimal.h"
#include "MMOSharedTypes.generated.h"

UENUM(BlueprintType)
enum class EMMOFaction : uint8
{
	None    UMETA(DisplayName = "None"),
	Alliance UMETA(DisplayName = "Alliance"),
	Horde    UMETA(DisplayName = "Horde"),
	Neutral  UMETA(DisplayName = "Neutral")
};

UENUM(BlueprintType)
enum class EMMOClass : uint8
{
	None    UMETA(DisplayName = "None"),
	Warrior UMETA(DisplayName = "Warrior"),
	Mage    UMETA(DisplayName = "Mage"),
	Rogue   UMETA(DisplayName = "Rogue"),
	Cleric  UMETA(DisplayName = "Cleric")
};

UENUM(BlueprintType)
enum class EMMOCharacterModel : uint8
{
	Human UMETA(DisplayName = "Human"),
	Elf  UMETA(DisplayName = "Elf"),
	Orc   UMETA(DisplayName = "Orc"),
	// add more as needed
};

USTRUCT(BlueprintType)
struct FMMOCharacterProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMMOFaction Faction = EMMOFaction::Alliance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMMOClass Class = EMMOClass::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMMOCharacterModel Model = EMMOCharacterModel::Human;

	UPROPERTY()
	bool bHasSavedLocation = false;

	UPROPERTY()
	FVector LastLocation = FVector::ZeroVector;
};
