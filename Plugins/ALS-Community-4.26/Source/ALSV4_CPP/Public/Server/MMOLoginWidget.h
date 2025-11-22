#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MMOLoginWidget.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;
class AMMOPlayerController;

UCLASS()
class ALSV4_CPP_API UMMOLoginWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// Called by PC so the widget can keep a typed username if you want
	void SetStatusMessage(const FString& InMessage);

protected:
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* UsernameTextBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PasswordTextBox;

	UPROPERTY(meta = (BindWidget))
	UButton* LoginButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateAccountButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* StatusText;

	// Button callbacks
	UFUNCTION()
	void HandleLoginClicked();

	UFUNCTION()
	void HandleCreateAccountClicked();

	AMMOPlayerController* GetMMOPlayerController() const;
};
