// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Networking/MMOPlayerController.h"
#include "Character/Networking/MMOCharacterCreationWidget.h"
#include "Character/Networking/MMOPlayerCharacter.h"
#include "Server/MMOLoginWidget.h"
#include "Server/MMOAccountStorage.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h" // for GEngine
#include "GameFramework/Pawn.h"
#include "Character/Networking/MMOPlayerState.h"

void AMMOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (LoginWidgetClass)
	{
		LoginWidget = CreateWidget<UMMOLoginWidget>(this, LoginWidgetClass);
		if (LoginWidget)
		{
			LoginWidget->AddToViewport(10);

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(LoginWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);

			bShowMouseCursor = true;
		}
	}

	// Ensure pawn starts hidden (server already set bIsHiddenForLogin = true,
	// but this helps if you test locally as listen server)
	if (AMMOPlayerCharacter* MMOChar = Cast<AMMOPlayerCharacter>(GetPawn()))
	{
		MMOChar->SetHiddenForLogin(true);
	}
}

void AMMOPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority()) // server side only
	{
		AMMOPlayerState* PS = GetPlayerState<AMMOPlayerState>();
		APawn* MyPawn = GetPawn();

		if (PS && MyPawn)
		{
			const FString& Username = PS->GetAccountUsername();

			if (!Username.IsEmpty())
			{
				UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
				UMMOAccountStorage* Storage = GI ? GI->GetSubsystem<UMMOAccountStorage>() : nullptr;

				if (Storage)
				{
					const FVector Loc = MyPawn->GetActorLocation();
					Storage->UpdateLastLocation(Username, Loc);
				}
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}


void AMMOPlayerController::SetCharacterProfile(
	const FString& InName,
	int32               InLevel,
	EMMOFaction         InFaction,
	EMMOClass           InClass,
	EMMOCharacterModel  InModel)
{
	AMMOPlayerState* PS = GetPlayerState<AMMOPlayerState>();
	if (!PS)
	{
		return;
	}

	if (IsLocalController())
	{
		// Client ? server RPC
		PS->ServerSetCharacterProfile(InName, InLevel, InFaction, InClass, InModel);
	}
	else
	{
		// Server-only path (listen server, etc.)
		PS->ApplyCharacterProfile(InName, InLevel, InFaction, InClass, InModel);
	}
}

void AMMOPlayerController::SetNameCommand(const FString& NewName)
{
	if (AMMOPlayerState* PS = GetPlayerState<AMMOPlayerState>())
	{
		PS->ServerSetCharacterProfile(
			NewName,
			PS->CharacterLevel,
			PS->Faction,
			PS->Class,
			PS->CharacterModel);
	}
}

void AMMOPlayerController::RequestLogin(const FString& Username, const FString& Password)
{
	ServerLogin(Username, Password);
}

void AMMOPlayerController::RequestCreateAccount(
	const FString& Username,
	const FString& Password,
	const FMMOCharacterProfile& Profile)
{
	ServerCreateAccount(Username, Password, Profile);
}

void AMMOPlayerController::ShowCharacterCreationScreen(const FString& OptionalUsername)
{
	if (!CharacterCreationWidgetClass)
	{
		return;
	}

	if (LoginWidget)
	{
		LoginWidget->RemoveFromParent();
		LoginWidget = nullptr;
	}

	if (!CreationWidget)
	{
		CreationWidget = CreateWidget<UMMOCharacterCreationWidget>(this, CharacterCreationWidgetClass);
		if (CreationWidget)
		{
			CreationWidget->AddToViewport(10);

			// Let it pre-fill username if we have one
			if (!OptionalUsername.IsEmpty())
			{
				CreationWidget->PrefillUsername(OptionalUsername);
			}

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(CreationWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);

			bShowMouseCursor = true;
		}
	}
}


//server side rpcs::

void AMMOPlayerController::ServerLogin_Implementation(const FString& Username, const FString& Password)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UMMOAccountStorage* Storage = GI ? GI->GetSubsystem<UMMOAccountStorage>() : nullptr;

	if (!Storage)
	{
		ClientOnLoginResult(false, TEXT("Login system unavailable."));
		return;
	}

	FMMOCharacterProfile Profile;
	FString Error;

	if (!Storage->TryLogin(Username, Password, Profile, Error))
	{
		ClientOnLoginResult(false, Error);
		return;
	}

	if (AMMOPlayerState* PS = GetPlayerState<AMMOPlayerState>())
	{
		// NEW: remember which account this player is tied to
		PS->SetAccountUsername(Username);

		PS->ApplyCharacterProfile(
			Profile.CharacterName,
			Profile.Level,
			Profile.Faction,
			Profile.Class,
			Profile.Model);
	}

	if (AMMOPlayerCharacter* MMOChar = Cast<AMMOPlayerCharacter>(GetPawn()))
	{
		MMOChar->SetHiddenForLogin(false);
		MMOChar->RefreshCharacterModelFromState();

		// NEW: spawn at saved location if we have one
		if (Profile.bHasSavedLocation)
		{
			MMOChar->SetActorLocation(Profile.LastLocation);
		}
	}

	ClientOnLoginResult(true, TEXT(""));
}


void AMMOPlayerController::ServerCreateAccount_Implementation(
	const FString& Username,
	const FString& Password,
	const FMMOCharacterProfile& Profile)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UMMOAccountStorage* Storage = GI ? GI->GetSubsystem<UMMOAccountStorage>() : nullptr;

	if (!Storage)
	{
		ClientOnCreateAccountResult(false, TEXT("Account system unavailable."));
		return;
	}

	FString Error;
	if (!Storage->TryCreateAccount(Username, Password, Profile, Error))
	{
		ClientOnCreateAccountResult(false, Error);
		return;
	}

	if (AMMOPlayerState* PS = GetPlayerState<AMMOPlayerState>())
	{
		PS->SetAccountUsername(Username); // NEW

		PS->ApplyCharacterProfile(
			Profile.CharacterName,
			Profile.Level,
			Profile.Faction,
			Profile.Class,
			Profile.Model);
	}

	if (AMMOPlayerCharacter* MMOChar = Cast<AMMOPlayerCharacter>(GetPawn()))
	{
		MMOChar->SetHiddenForLogin(false);
		MMOChar->RefreshCharacterModelFromState();

		// On first create, you might want a spawn point instead of Profile.LastLocation.
		// For now, Profile likely has default bHasSavedLocation = false, so this won't run.
	}

	ClientOnCreateAccountResult(true, TEXT(""));
}


void AMMOPlayerController::ClientOnLoginResult_Implementation(bool bSuccess, const FString& ErrorMessage)
{
	// TEMP DEBUG
	if (GEngine)
	{
		const FString Msg = bSuccess
			? TEXT("Login success!")
			: FString::Printf(TEXT("Login FAILED: %s"), *ErrorMessage);

		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			bSuccess ? FColor::Green : FColor::Red,
			Msg);
	}

	if (!bSuccess)
	{
		// later this is where you'll set a status text on the login widget
		return;
	}

	// Success -> remove login UI and return control to game
	if (LoginWidget)
	{
		LoginWidget->RemoveFromParent();
		LoginWidget = nullptr;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void AMMOPlayerController::ClientOnCreateAccountResult_Implementation(bool bSuccess, const FString& ErrorMessage)
{
	// TEMP DEBUG
	if (GEngine)
	{
		const FString Msg = bSuccess
			? TEXT("Account created successfully!")
			: FString::Printf(TEXT("CreateAccount FAILED: %s"), *ErrorMessage);

		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			bSuccess ? FColor::Cyan : FColor::Red,
			Msg);
	}

	if (!bSuccess)
	{
		// later: surface ErrorMessage in the creation widget
		return;
	}

	if (CreationWidget)
	{
		CreationWidget->RemoveFromParent();
		CreationWidget = nullptr;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}
