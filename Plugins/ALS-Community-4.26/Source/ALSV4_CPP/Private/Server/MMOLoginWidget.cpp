// Fill out your copyright notice in the Description page of Project Settings.


#include "Server/MMOLoginWidget.h"

#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Character/Networking/MMOPlayerController.h"

void UMMOLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &UMMOLoginWidget::HandleLoginClicked);
	}

	if (CreateAccountButton)
	{
		CreateAccountButton->OnClicked.AddDynamic(this, &UMMOLoginWidget::HandleCreateAccountClicked);
	}
}

AMMOPlayerController* UMMOLoginWidget::GetMMOPlayerController() const
{
	return Cast<AMMOPlayerController>(GetOwningPlayer());
}

void UMMOLoginWidget::HandleLoginClicked()
{
	AMMOPlayerController* PC = GetMMOPlayerController();
	if (!PC) return;

	const FString Username = UsernameTextBox
		? UsernameTextBox->GetText().ToString().TrimStartAndEnd()
		: TEXT("");

	const FString Password = PasswordTextBox
		? PasswordTextBox->GetText().ToString()
		: TEXT("");

	PC->RequestLogin(Username, Password);
}

void UMMOLoginWidget::HandleCreateAccountClicked()
{
	AMMOPlayerController* PC = GetMMOPlayerController();
	if (!PC) return;

	const FString Username = UsernameTextBox
		? UsernameTextBox->GetText().ToString().TrimStartAndEnd()
		: TEXT("");

	PC->ShowCharacterCreationScreen(Username);
}

void UMMOLoginWidget::SetStatusMessage(const FString& InMessage)
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(InMessage));
	}
}
