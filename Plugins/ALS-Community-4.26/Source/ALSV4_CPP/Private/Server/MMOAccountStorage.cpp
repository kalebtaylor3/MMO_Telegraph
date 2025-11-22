// MMOAccountStorage.cpp

#include "Server/MMOAccountStorage.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "Misc/Guid.h"
#include "HAL/FileManager.h"

#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "GameFramework/WorldSettings.h"

void UMMOAccountStorage::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();

	bIsServerStorage = (World && World->GetNetMode() != NM_Client);

	if (!bIsServerStorage)
	{
		UE_LOG(LogTemp, Log, TEXT("UMMOAccountStorage: client instance, no disk I/O."));
		return;
	}

	AccountsFilePath = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("Accounts"),
		TEXT("MMOAccounts.json"));

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AccountsFilePath), true);

	UE_LOG(LogTemp, Log, TEXT("UMMOAccountStorage::Initialize (SERVER) - DB path: %s"), *AccountsFilePath);

	LoadFromDisk();

	UE_LOG(LogTemp, Log, TEXT("UMMOAccountStorage::Initialize - Loaded %d accounts"), Accounts.Num());
}



void UMMOAccountStorage::Deinitialize()
{
	if (bIsServerStorage)
	{
		SaveToDisk();
	}
	Super::Deinitialize();
}

FString UMMOAccountStorage::NormalizeUsername(const FString& In)
{
	return In.TrimStartAndEnd().ToLower();
}

FString UMMOAccountStorage::GenerateSalt() const
{
	return FGuid::NewGuid().ToString(EGuidFormats::Digits);
}

FString UMMOAccountStorage::HashPassword(const FString& Password, const FString& Salt) const
{
	const FString Combined = Password + TEXT(":") + Salt;
	return FMD5::HashAnsiString(*Combined);
}

void UMMOAccountStorage::LoadFromDisk()
{
	Accounts.Empty();

	if (!IFileManager::Get().FileExists(*AccountsFilePath))
	{
		return; // nothing yet
	}

	FString FileContents;
	if (!FFileHelper::LoadFileToString(FileContents, *AccountsFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMMOAccountStorage: Failed to read %s"), *AccountsFilePath);
		return;
	}

	TSharedPtr<FJsonObject> RootObj;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);

	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMMOAccountStorage: Failed to parse JSON %s"), *AccountsFilePath);
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* AccountsArray;
	if (!RootObj->TryGetArrayField(TEXT("accounts"), AccountsArray))
	{
		return;
	}

	for (const TSharedPtr<FJsonValue>& Val : *AccountsArray)
	{
		const TSharedPtr<FJsonObject> AccObj = Val->AsObject();
		if (!AccObj.IsValid())
		{
			continue;
		}

		FMMOAccountRecord Rec;
		Rec.Username = AccObj->GetStringField(TEXT("username"));
		Rec.PasswordHash = AccObj->GetStringField(TEXT("passwordHash"));
		Rec.Salt = AccObj->GetStringField(TEXT("salt"));

		const TSharedPtr<FJsonObject>* ProfileObj;
		if (AccObj->TryGetObjectField(TEXT("profile"), ProfileObj))
		{
			Rec.Profile.CharacterName = (*ProfileObj)->GetStringField(TEXT("name"));
			Rec.Profile.Level = (*ProfileObj)->GetIntegerField(TEXT("level"));
			Rec.Profile.Faction = static_cast<EMMOFaction>((*ProfileObj)->GetIntegerField(TEXT("faction")));
			Rec.Profile.Class = static_cast<EMMOClass>((*ProfileObj)->GetIntegerField(TEXT("class")));
			Rec.Profile.Model = static_cast<EMMOCharacterModel>((*ProfileObj)->GetIntegerField(TEXT("model")));

			// NEW: location (backwards compatible)
			bool bHasLoc = false;
			if ((*ProfileObj)->TryGetBoolField(TEXT("hasLocation"), bHasLoc) && bHasLoc)
			{
				double X = 0, Y = 0, Z = 0;
				(*ProfileObj)->TryGetNumberField(TEXT("locX"), X);
				(*ProfileObj)->TryGetNumberField(TEXT("locY"), Y);
				(*ProfileObj)->TryGetNumberField(TEXT("locZ"), Z);

				Rec.Profile.bHasSavedLocation = true;
				Rec.Profile.LastLocation = FVector((float)X, (float)Y, (float)Z);
			}
			else
			{
				Rec.Profile.bHasSavedLocation = false;
				Rec.Profile.LastLocation = FVector::ZeroVector;
			}
		}

		Accounts.Add(NormalizeUsername(Rec.Username), Rec);
	}


	UE_LOG(LogTemp, Log, TEXT("UMMOAccountStorage: Loaded %d accounts"), Accounts.Num());
}

void UMMOAccountStorage::SaveToDisk() const
{
	if (!bIsServerStorage)
	{
		return; // clients never write
	}

	TSharedRef<FJsonObject> RootObj = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> AccountsArray;

	for (const TPair<FString, FMMOAccountRecord>& Pair : Accounts)
	{
		const FMMOAccountRecord& Rec = Pair.Value;

		TSharedRef<FJsonObject> AccObj = MakeShared<FJsonObject>();
		AccObj->SetStringField(TEXT("username"), Rec.Username);
		AccObj->SetStringField(TEXT("passwordHash"), Rec.PasswordHash);
		AccObj->SetStringField(TEXT("salt"), Rec.Salt);

		TSharedRef<FJsonObject> ProfileObj = MakeShared<FJsonObject>();
		ProfileObj->SetStringField(TEXT("name"), Rec.Profile.CharacterName);
		ProfileObj->SetNumberField(TEXT("level"), Rec.Profile.Level);
		ProfileObj->SetNumberField(TEXT("faction"), (int32)Rec.Profile.Faction);
		ProfileObj->SetNumberField(TEXT("class"), (int32)Rec.Profile.Class);
		ProfileObj->SetNumberField(TEXT("model"), (int32)Rec.Profile.Model);

		ProfileObj->SetBoolField(TEXT("hasLocation"), Rec.Profile.bHasSavedLocation);
		if (Rec.Profile.bHasSavedLocation)
		{
			ProfileObj->SetNumberField(TEXT("locX"), Rec.Profile.LastLocation.X);
			ProfileObj->SetNumberField(TEXT("locY"), Rec.Profile.LastLocation.Y);
			ProfileObj->SetNumberField(TEXT("locZ"), Rec.Profile.LastLocation.Z);
		}

		AccObj->SetObjectField(TEXT("profile"), ProfileObj);

		AccountsArray.Add(MakeShared<FJsonValueObject>(AccObj));
	}

	RootObj->SetArrayField(TEXT("accounts"), AccountsArray);

	FString OutString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutString);
	FJsonSerializer::Serialize(RootObj, Writer);

	if (!FFileHelper::SaveStringToFile(OutString, *AccountsFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMMOAccountStorage: Failed to save %s"), *AccountsFilePath);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("UMMOAccountStorage: Saved %d accounts to %s"),
			Accounts.Num(), *AccountsFilePath);
	}
}


bool UMMOAccountStorage::TryLogin(
	const FString& Username,
	const FString& Password,
	FMMOCharacterProfile& OutProfile,
	FString& OutError)
{

	if (!bIsServerStorage)
	{
		OutError = TEXT("Login is not available on this instance.");
		return false;
	}

	const FString Key = NormalizeUsername(Username);
	FMMOAccountRecord* Rec = Accounts.Find(Key);
	if (!Rec)
	{
		OutError = TEXT("Unknown account.");
		return false;
	}

	const FString ExpectedHash = Rec->PasswordHash;
	const FString ActualHash = HashPassword(Password, Rec->Salt);

	if (!ExpectedHash.Equals(ActualHash, ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Incorrect password.");
		return false;
	}

	OutProfile = Rec->Profile;
	return true;
}

bool UMMOAccountStorage::TryCreateAccount(
	const FString& Username,
	const FString& Password,
	const FMMOCharacterProfile& Profile,
	FString& OutError)
{

	if (!bIsServerStorage)
	{
		OutError = TEXT("Account storage is not authoritative on this instance.");
		return false;
	}

	const FString Key = NormalizeUsername(Username);

	if (Username.IsEmpty())
	{
		OutError = TEXT("Username required.");
		return false;
	}

	if (Password.Len() < 4)
	{
		OutError = TEXT("Password too short.");
		return false;
	}

	if (Accounts.Contains(Key))
	{
		OutError = TEXT("Username already taken.");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("TryCreateAccount: BEFORE add, Accounts.Num = %d"), Accounts.Num());

	FMMOAccountRecord Rec;
	Rec.Username = Username;
	Rec.Salt = GenerateSalt();
	Rec.PasswordHash = HashPassword(Password, Rec.Salt);
	Rec.Profile = Profile;

	Accounts.Add(Key, Rec);

	UE_LOG(LogTemp, Log, TEXT("TryCreateAccount: AFTER add, Accounts.Num = %d"), Accounts.Num());

	SaveToDisk();

	return true;
}

bool UMMOAccountStorage::UpdateLastLocation(const FString& Username, const FVector& NewLocation)
{
	if (!bIsServerStorage)
	{
		return false;
	}

	const FString Key = NormalizeUsername(Username);
	FMMOAccountRecord* Rec = Accounts.Find(Key);
	if (!Rec)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateLastLocation: no account '%s'"), *Username);
		return false;
	}

	Rec->Profile.bHasSavedLocation = true;
	Rec->Profile.LastLocation = NewLocation;

	SaveToDisk();
	return true;
}

