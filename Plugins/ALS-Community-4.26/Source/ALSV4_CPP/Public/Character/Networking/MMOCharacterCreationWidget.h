#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/Networking/MMOSharedTypes.h"
#include "MMOCharacterCreationWidget.generated.h"

class UEditableTextBox;
class UComboBoxString;
class UButton;
class AMMOPlayerController;

UCLASS()
class ALSV4_CPP_API UMMOCharacterCreationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void PrefillUsername(const FString& InUsername);

protected:
	// These must match the widget names in the UMG BP
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* NameTextBox;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* FactionComboBox;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ClassComboBox;

	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ModelComboBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PasswordTextBox;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UEditableTextBox* UsernameTextBox;

	// Called when ConfirmButton is clicked
	UFUNCTION()
	void HandleConfirmClicked();

	EMMOFaction GetSelectedFaction() const;
	EMMOClass   GetSelectedClass() const;
	EMMOCharacterModel GetSelectedModel() const;
};
