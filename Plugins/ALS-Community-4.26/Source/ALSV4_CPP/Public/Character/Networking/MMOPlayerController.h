#pragma once

#include "CoreMinimal.h"
#include "Character/ALSPlayerController.h"
#include "Character/Networking/MMOPlayerState.h"
#include "Character/Networking/MMOSharedTypes.h"
#include "Character/MMOSelectionCircle.h"
#include "MMOPlayerController.generated.h"

class AMMOPlayerState;
class UMMOLoginWidget;
class UMMOCharacterCreationWidget;
class UInputAction;

/**
 * MMO-style player controller that can configure character profile on the server.
 */
UCLASS()
class ALSV4_CPP_API AMMOPlayerController : public AALSPlayerController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

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

	UPROPERTY(EditDefaultsOnly, Category = "MMO|Input")
	UInputAction* SelectAction;

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

	// === HOVER HIGHLIGHT ===

	/** Current actor under the mouse (for this client only) */
	UPROPERTY()
	AActor* CurrentHoverActor = nullptr;

	UPROPERTY()
	AActor* CurrentSelectedActor = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "MMO|Selection")
	TSubclassOf<class AMMOSelectionCircle> SelectionCircleClass;

	UPROPERTY()
	AMMOSelectionCircle* SelectionCircle = nullptr;

	/** MPC used by the post-process outline material */
	UPROPERTY(EditDefaultsOnly, Category = "MMO|Highlight")
	UMaterialParameterCollection* HighlightMPC;

	/** Called each frame on the local client to update hover highlighting */
	void UpdateHoverHighlight(float DeltaSeconds);

	/** Clear highlight from CurrentHoverActor */
	void ClearHoverHighlight();

	/** Apply highlight to this actor (and set CurrentHoverActor) */
	void ApplyHighlightToActor(AActor* NewActor);

	/** Set MPC outline color based on actor type (enemy/player/npc) */
	void SetOutlineColorForActor(AActor* Actor);

	// Input handlers
	void HandleSelectPressed();

	// Selection helpers
	void ClearSelection();
	void ApplySelectionToActor(AActor* NewSelection);
};
