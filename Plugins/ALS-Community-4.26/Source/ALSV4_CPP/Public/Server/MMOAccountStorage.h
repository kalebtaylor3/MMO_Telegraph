// MMOAccountStorage.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Character/Networking/MMOSharedTypes.h"
#include "MMOAccountStorage.generated.h"

USTRUCT()
struct FMMOAccountRecord
{
	GENERATED_BODY()

	UPROPERTY()
	FString Username;

	UPROPERTY()
	FString PasswordHash;

	UPROPERTY()
	FString Salt;

	UPROPERTY()
	FMMOCharacterProfile Profile;
};

UCLASS()
class ALSV4_CPP_API UMMOAccountStorage : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Called on SERVER: validate username+password and output profile if OK */
	bool TryLogin(
		const FString& Username,
		const FString& Password,
		FMMOCharacterProfile& OutProfile,
		FString& OutError);

	/** Called on SERVER: create a new account if username free */
	bool TryCreateAccount(
		const FString& Username,
		const FString& Password,
		const FMMOCharacterProfile& Profile,
		FString& OutError);

	bool UpdateLastLocation(const FString& Username, const FVector& NewLocation);

private:
	UPROPERTY()
	TMap<FString, FMMOAccountRecord> Accounts; // key = Username (lowercase)

	FString AccountsFilePath;

	// NEW: only the authoritative server instance should read/write disk
	bool bIsServerStorage = false;

	void LoadFromDisk();
	void SaveToDisk() const;

	FString HashPassword(const FString& Password, const FString& Salt) const;
	FString GenerateSalt() const;

	static FString NormalizeUsername(const FString& In);
};
