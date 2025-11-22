#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Character/Networking/MMOSharedTypes.h"
#include "MMOPlayerState.generated.h"

UCLASS()
class ALSV4_CPP_API AMMOPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMMOPlayerState();

	/** Character display name (what we’ll show over heads / in UI) */
	UPROPERTY(ReplicatedUsing = OnRep_CharacterName, BlueprintReadOnly, Category = "MMO")
	FString CharacterName;

	/** Character level */
	UPROPERTY(ReplicatedUsing = OnRep_CharacterLevel, BlueprintReadOnly, Category = "MMO")
	int32 CharacterLevel;

	/** Faction (Alliance / Horde / etc.) */
	UPROPERTY(ReplicatedUsing = OnRep_Faction, BlueprintReadOnly, Category = "MMO")
	EMMOFaction Faction;

	/** Class (Warrior / Mage / etc.) */
	UPROPERTY(ReplicatedUsing = OnRep_Class, BlueprintReadOnly, Category = "MMO")
	EMMOClass Class;

	UPROPERTY(ReplicatedUsing = OnRep_CharacterModel, BlueprintReadOnly, Category = "MMO")
	EMMOCharacterModel CharacterModel;

	UPROPERTY(VisibleAnywhere, Category = "MMO|Account")
	FString AccountUsername;

	void SetAccountUsername(const FString& InUsername)
	{
		AccountUsername = InUsername;
	}

	const FString& GetAccountUsername() const { return AccountUsername; }

	/** Initial setup from client ? server (called by client, runs on server, then replicates) */
	void ApplyCharacterProfile(
		const FString& InName,
		int32               InLevel,
		EMMOFaction         InFaction,
		EMMOClass           InClass,
		EMMOCharacterModel  InModel);

	UFUNCTION(Server, Reliable)
	void ServerSetCharacterProfile(
		const FString& InName,
		int32               InLevel,
		EMMOFaction         InFaction,
		EMMOClass           InClass,
		EMMOCharacterModel  InModel);

protected:
	// Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// OnRep callbacks so UI / nameplates can refresh if needed
	UFUNCTION()
	void OnRep_CharacterName();

	UFUNCTION()
	void OnRep_CharacterLevel();

	UFUNCTION()
	void OnRep_Faction();

	UFUNCTION()
	void OnRep_Class();

	UFUNCTION()
	void OnRep_CharacterModel();
};
