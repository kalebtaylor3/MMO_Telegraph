#pragma once

#include "CoreMinimal.h"
#include "Character/ALSPlayerController.h"
#include "Character/Networking/MMOPlayerState.h"
#include "Character/Networking/MMOSharedTypes.h"
#include "MMOPlayerController.generated.h"

class AMMOPlayerState;
class UMMOLoginWidget;
class UMMOCharacterCreationWidget;

/**
 * MMO-style player controller that can configure character profile on the server.
 */
UCLASS()
class ALSV4_CPP_API AMMOPlayerController : public AALSPlayerController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(Exec)
	void SetNameCommand(const FString& NewName);

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void RequestLogin(const FString& Username, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void ShowCharacterCreationScreen(const FString& OptionalUsername);

	UFUNCTION(BlueprintCallable, Category = "MMO|Login")
	void RequestCreateAccount(
		const FString& Username,
		const FString& Password,
		const FMMOCharacterProfile& Profile);

	// Helper to request a profile change from the local client
	UFUNCTION(BlueprintCallable, Category = "MMO")
	void SetCharacterProfile(
		const FString& InName,
		int32               InLevel,
		EMMOFaction         InFaction,
		EMMOClass           InClass,
		EMMOCharacterModel  InModel);

protected:


	UPROPERTY(EditDefaultsOnly, Category = "MMO|UI")
	TSubclassOf<UMMOLoginWidget> LoginWidgetClass;

	// UI class for character creation
	UPROPERTY(EditDefaultsOnly, Category = "MMO|UI")
	TSubclassOf<UMMOCharacterCreationWidget> CharacterCreationWidgetClass;

	UPROPERTY()
	UMMOLoginWidget* LoginWidget;

	UPROPERTY()
	UMMOCharacterCreationWidget* CreationWidget;

	// === Server RPCs ===
	UFUNCTION(Server, Reliable)
	void ServerLogin(const FString& Username, const FString& Password);

	UFUNCTION(Server, Reliable)
	void ServerCreateAccount(const FString& Username, const FString& Password, const FMMOCharacterProfile& Profile);

	// === Client callbacks ===
	UFUNCTION(Client, Reliable)
	void ClientOnLoginResult(bool bSuccess, const FString& ErrorMessage);

	UFUNCTION(Client, Reliable)
	void ClientOnCreateAccountResult(bool bSuccess, const FString& ErrorMessage);
};
