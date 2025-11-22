// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Networking/MMOPlayerCharacter.h"

#include "Kismet/KismetMathLibrary.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h" 
#include "Character/Networking/MMONameplateWidget.h"
#include "Character/Networking/MMOPlayerState.h"

AMMOPlayerCharacter::AMMOPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	PrimaryActorTick.bCanEverTick = true;

	// Default to "hidden while in login"
	bIsHiddenForLogin = true;

	// Create nameplate widget component
	NameplateWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameplateWidget"));
	NameplateWidgetComponent->SetupAttachment(GetMesh()); // attach to mesh so it follows animations

	// Basic widget config
	NameplateWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	NameplateWidgetComponent->SetDrawAtDesiredSize(true);
	NameplateWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f)); // tweak height

	CharacterOverlayMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterOverlayMesh"));
	CharacterOverlayMesh->SetupAttachment(GetMesh());
	CharacterOverlayMesh->SetRelativeLocation(FVector::ZeroVector);
	CharacterOverlayMesh->SetRelativeRotation(FRotator::ZeroRotator);
	CharacterOverlayMesh->SetRelativeScale3D(FVector::OneVector);

}

void AMMOPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMMOPlayerCharacter, bIsHiddenForLogin);
}


void AMMOPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bFaceCamera || !NameplateWidgetComponent)
	{
		return;
	}

	// Get local player controller for this world (each client has its own)
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	const FVector NameplateLocation = NameplateWidgetComponent->GetComponentLocation();

	// Look from nameplate to camera
	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(NameplateLocation, CameraLocation);

	// Only yaw (optional): keeps it upright like most MMOs
	LookAtRot.Pitch = 0.f;
	LookAtRot.Roll = 0.f;

	NameplateWidgetComponent->SetWorldRotation(LookAtRot);
}

void AMMOPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CharacterOverlayMesh && GetMesh())
	{
		CharacterOverlayMesh->SetLeaderPoseComponent(GetMesh());
	}

	// Force login-hidden state at spawn
	if (HasAuthority())
	{
		bIsHiddenForLogin = true;
	}
	ApplyHiddenForLogin();

	InitializeNameplate();

	// This is safe now because ApplyCharacterModel respects bIsHiddenForLogin
	RefreshCharacterModelFromState();
}

void AMMOPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitializeNameplate();
	RefreshCharacterModelFromState();
}

void AMMOPlayerCharacter::InitializeNameplate()
{
	if (!NameplateWidgetComponent)
	{
		return;
	}

	// If this pawn is locally controlled on THIS client, hide its own nameplate.
	// Remote players on this client will still show theirs.
	if (IsLocallyControlled())
	{
		NameplateWidgetComponent->SetHiddenInGame(true);
		NameplateWidgetComponent->SetVisibility(false, true);
		return; // don't bother wiring the widget for the local player
	}

	// From here on, this path runs on non-local pawns (i.e., other players)
	if (!NameplateWidgetComponent->GetWidgetClass() && NameplateWidgetClass)
	{
		NameplateWidgetComponent->SetWidgetClass(NameplateWidgetClass);
	}

	AMMOPlayerState* PS = GetPlayerState<AMMOPlayerState>();
	if (!PS)
	{
		return;
	}

	if (!NameplateWidgetComponent->GetUserWidgetObject())
	{
		NameplateWidgetComponent->InitWidget();
	}

	if (UUserWidget* RawWidget = NameplateWidgetComponent->GetUserWidgetObject())
	{
		if (UMMONameplateWidget* Nameplate = Cast<UMMONameplateWidget>(RawWidget))
		{
			Nameplate->InitializeFromPlayerState(PS);
		}
	}
}

// ===== Login hiding =====

void AMMOPlayerCharacter::ApplyHiddenForLogin()
{
	const bool bHide = bIsHiddenForLogin;

	// Refresh overlay so it respects the new flag
	RefreshCharacterModelFromState();

	// Nameplate matches login state
	if (NameplateWidgetComponent)
	{
		NameplateWidgetComponent->SetHiddenInGame(bHide);
		NameplateWidgetComponent->SetVisibility(!bHide, true);
	}
}

void AMMOPlayerCharacter::OnRep_IsHiddenForLogin()
{
	ApplyHiddenForLogin();
}

void AMMOPlayerCharacter::SetHiddenForLogin(bool bNewHidden)
{
	// Only the server should drive this flag
	if (!HasAuthority())
	{
		return;
	}

	if (bIsHiddenForLogin == bNewHidden)
	{
		return;
	}

	bIsHiddenForLogin = bNewHidden;

	ApplyHiddenForLogin();   // this will also RefreshCharacterModelFromState()
}

void AMMOPlayerCharacter::RefreshCharacterModelFromState()
{
	AMMOPlayerState* PS = GetPlayerState<AMMOPlayerState>();
	if (!PS)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("RefreshCharacterModelFromState: %s, Model=%d"),
		*GetName(), (int32)PS->CharacterModel);

	ApplyCharacterModel(PS->CharacterModel);
}

void AMMOPlayerCharacter::ApplyCharacterModel(EMMOCharacterModel Model)
{
	if (!CharacterOverlayMesh)
	{
		return;
	}

	USkeletalMesh* MeshToUse = nullptr;

	switch (Model)
	{
	case EMMOCharacterModel::Human:
		MeshToUse = HumanMesh;
		break;
	case EMMOCharacterModel::Elf:
		MeshToUse = ElfMesh;
		break;
	case EMMOCharacterModel::Orc:
		MeshToUse = OrcMesh;
		break;
	default:
		MeshToUse = HumanMesh;
		break;
	}

	CharacterOverlayMesh->SetSkeletalMesh(MeshToUse);

	// *** THIS is the important part ***
	const bool bShouldBeVisible = (MeshToUse != nullptr) && !bIsHiddenForLogin;

	CharacterOverlayMesh->SetHiddenInGame(!bShouldBeVisible, true);
	CharacterOverlayMesh->SetVisibility(bShouldBeVisible, true);
}

