#include "Enemy/MMOMobCharacter.h"   // adjust path if needed

#include "Components/SphereComponent.h"
#include "Enemy/MMOMobAIController.h"
#include "Kismet/GameplayStatics.h"

AMMOMobCharacter::AMMOMobCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	// Use our custom AI controller
	AIControllerClass = AMMOMobAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AggroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AggroSphere"));
	AggroSphere->SetupAttachment(RootComponent);

	// Basic collision setup; we can redo in BeginPlay but this ensures it's valid
	AggroSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AggroSphere->SetCollisionObjectType(ECC_WorldDynamic);
	AggroSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AggroSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AggroSphere->SetGenerateOverlapEvents(true);
}

void AMMOMobCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	if (!AggroSphere)
	{
		UE_LOG(LogTemp, Error, TEXT("MMOMobCharacter '%s': AggroSphere is null in BeginPlay!"), *GetName());
		return; // bail out instead of crashing
	}

	// Just in case some blueprint messed with it, re-apply everything
	AggroSphere->SetSphereRadius(AggroRadius);
	AggroSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AggroSphere->SetCollisionObjectType(ECC_WorldDynamic);
	AggroSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AggroSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AggroSphere->SetGenerateOverlapEvents(true);

	// Bind overlap
	AggroSphere->OnComponentBeginOverlap.AddDynamic(
		this, &AMMOMobCharacter::OnAggroSphereBeginOverlap);

	UE_LOG(LogTemp, Log, TEXT("MMOMobCharacter '%s' BeginPlay: AggroRadius=%.1f, SpawnLocation=%s"),
		*GetName(), AggroRadius, *SpawnLocation.ToString());
}

void AMMOMobCharacter::OnAggroSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32                OtherBodyIndex,
	bool                 bFromSweep,
	const FHitResult& SweepResult)
{
	// Only the server should decide aggro
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// Ignore other mobs (including subclasses of AMMOMobCharacter)
	if (OtherActor->IsA(AMMOMobCharacter::StaticClass()))
	{
		return;
	}

	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn)
	{
		return;
	}

	// Only aggro things that are actually player controlled
	if (!OtherPawn->IsPlayerControlled())
	{
		return;
	}

	if (AMMOMobAIController* MobAI = Cast<AMMOMobAIController>(GetController()))
	{
		MobAI->NotifyAggro(OtherPawn);
	}
}


void AMMOMobCharacter::SetAIState(EMMOMobAIState NewState)
{
	if (AIState == NewState)
	{
		return;
	}

	AIState = NewState;

	UE_LOG(LogTemp, Verbose, TEXT("MMOMobCharacter '%s' state -> %d"), *GetName(), (int32)AIState);
}

void AMMOMobCharacter::PerformBasicAttack()
{
	// For now just log – this is where telegraphs will hang off
	UE_LOG(LogTemp, Log, TEXT("Mob %s performs basic attack!"), *GetName());
}
