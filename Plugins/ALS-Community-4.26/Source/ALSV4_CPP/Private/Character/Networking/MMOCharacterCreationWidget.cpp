// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Networking/MMOCharacterCreationWidget.h"

#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

#include "Character/Networking/MMOPlayerController.h"
#include "Character/Networking/MMOPlayerState.h"

void UMMOCharacterCreationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Populate combo boxes if they are empty
	if (FactionComboBox && FactionComboBox->GetOptionCount() == 0)
	{
		FactionComboBox->AddOption(TEXT("Alliance"));
		FactionComboBox->AddOption(TEXT("Horde"));
		FactionComboBox->AddOption(TEXT("Neutral"));
		FactionComboBox->SetSelectedIndex(0);
	}

	if (ClassComboBox && ClassComboBox->GetOptionCount() == 0)
	{
		ClassComboBox->AddOption(TEXT("Warrior"));
		ClassComboBox->AddOption(TEXT("Mage"));
		ClassComboBox->AddOption(TEXT("Rogue"));
		ClassComboBox->SetSelectedIndex(0);
	}

	if (ModelComboBox && ModelComboBox->GetOptionCount() == 0)
	{
		ModelComboBox->AddOption(TEXT("Human"));
		ModelComboBox->AddOption(TEXT("Elf"));
		ModelComboBox->AddOption(TEXT("Orc"));
		ModelComboBox->SetSelectedIndex(0);
	}

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UMMOCharacterCreationWidget::HandleConfirmClicked);
	}
}

void UMMOCharacterCreationWidget::PrefillUsername(const FString& InUsername)
{
	if (UsernameTextBox)
	{
		UsernameTextBox->SetText(FText::FromString(InUsername));
	}

	if (NameTextBox && NameTextBox->GetText().IsEmpty())
	{
		NameTextBox->SetText(FText::FromString(InUsername));
	}
}

void UMMOCharacterCreationWidget::HandleConfirmClicked()
{
	AMMOPlayerController* PC = Cast<AMMOPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		return;
	}

	// Account username
	FString AccountName = TEXT("Player");
	if (UsernameTextBox)
	{
		const FString EnteredUser = UsernameTextBox->GetText().ToString().TrimStartAndEnd();
		if (!EnteredUser.IsEmpty())
		{
			AccountName = EnteredUser;
		}
	}
	else if (NameTextBox)
	{
		AccountName = NameTextBox->GetText().ToString().TrimStartAndEnd();
	}

	// Character name (this is what we show above head)
	FMMOCharacterProfile Profile;
	Profile.CharacterName = TEXT("Player");
	if (NameTextBox)
	{
		const FString Entered = NameTextBox->GetText().ToString().TrimStartAndEnd();
		if (!Entered.IsEmpty())
		{
			Profile.CharacterName = Entered;
		}
	}

	Profile.Level = 1;
	Profile.Faction = GetSelectedFaction();
	Profile.Class = GetSelectedClass();
	Profile.Model = GetSelectedModel();

	// Password
	FString Password = TEXT("");
	if (PasswordTextBox)
	{
		Password = PasswordTextBox->GetText().ToString();
	}

	// Ask controller to create the account on the server
	PC->RequestCreateAccount(AccountName, Password, Profile);

	// UI cleanup – controller will show/hide as needed on success/fail
}

EMMOFaction UMMOCharacterCreationWidget::GetSelectedFaction() const
{
	if (!FactionComboBox)
	{
		return EMMOFaction::Alliance;
	}

	const FString Selected = FactionComboBox->GetSelectedOption();
	if (Selected == TEXT("Horde"))
	{
		return EMMOFaction::Horde;
	}
	if (Selected == TEXT("Neutral"))
	{
		return EMMOFaction::Neutral;
	}
	return EMMOFaction::Alliance;
}

EMMOClass UMMOCharacterCreationWidget::GetSelectedClass() const
{
	if (!ClassComboBox)
	{
		return EMMOClass::Warrior;
	}

	const FString Selected = ClassComboBox->GetSelectedOption();
	if (Selected == TEXT("Mage"))
	{
		return EMMOClass::Mage;
	}
	if (Selected == TEXT("Rogue"))
	{
		return EMMOClass::Rogue;
	}
	return EMMOClass::Warrior;
}

EMMOCharacterModel UMMOCharacterCreationWidget::GetSelectedModel() const
{
	if (!ModelComboBox)
	{
		return EMMOCharacterModel::Human;
	}

	const FString Selected = ModelComboBox->GetSelectedOption();

	if (Selected == TEXT("Elf"))
	{
		return EMMOCharacterModel::Elf;
	}
	if (Selected == TEXT("Orc"))
	{
		return EMMOCharacterModel::Orc;
	}
	return EMMOCharacterModel::Human;
}

