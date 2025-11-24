// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MMOSelectionCircle.h"

#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AMMOSelectionCircle::AMMOSelectionCircle()
{
	PrimaryActorTick.bCanEverTick = false;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("SelectionDecal"));
	RootComponent = DecalComp;

	// X = projection depth, Y/Z = width/height
	DecalComp->DecalSize = FVector(32.f, 24.f, 24.f);

	// SUPER IMPORTANT: point decal DOWN onto the floor
	DecalComp->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	// Don’t fade with distance
	DecalComp->SetFadeScreenSize(0.f);

	// Start hidden
	DecalComp->SetVisibility(false);
	SetActorHiddenInGame(true);
}

void AMMOSelectionCircle::BeginPlay()
{
	Super::BeginPlay();

	if (DecalComp && DecalComp->GetMaterial(0))
	{
		DynMaterial = DecalComp->CreateDynamicMaterialInstance();
	}
}

void AMMOSelectionCircle::SetActive(bool bInActive)
{
	if (DecalComp)
	{
		DecalComp->SetVisibility(bInActive);
		SetActorHiddenInGame(!bInActive);
	}
}

void AMMOSelectionCircle::SetColor(const FLinearColor& InColor)
{
	if (DynMaterial)
	{
		// Your decal material needs a Vector parameter named "TintColor"
		DynMaterial->SetVectorParameterValue(TEXT("TintColor"), InColor);
	}
}

