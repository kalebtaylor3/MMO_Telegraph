// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSPlayerCameraManager.h"

#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/ALSPlayerController.h"
#include "Character/Animation/ALSPlayerCameraBehavior.h"
#include "Components/ALSDebugComponent.h"

#include "Kismet/KismetMathLibrary.h"


const FName NAME_CameraBehavior(TEXT("CameraBehavior"));
const FName NAME_CameraOffset_X(TEXT("CameraOffset_X"));
const FName NAME_CameraOffset_Y(TEXT("CameraOffset_Y"));
const FName NAME_CameraOffset_Z(TEXT("CameraOffset_Z"));
const FName NAME_Override_Debug(TEXT("Override_Debug"));
const FName NAME_PivotLagSpeed_X(TEXT("PivotLagSpeed_X"));
const FName NAME_PivotLagSpeed_Y(TEXT("PivotLagSpeed_Y"));
const FName NAME_PivotLagSpeed_Z(TEXT("PivotLagSpeed_Z"));
const FName NAME_PivotOffset_X(TEXT("PivotOffset_X"));
const FName NAME_PivotOffset_Y(TEXT("PivotOffset_Y"));
const FName NAME_PivotOffset_Z(TEXT("PivotOffset_Z"));
const FName NAME_RotationLagSpeed(TEXT("RotationLagSpeed"));
const FName NAME_Weight_FirstPerson(TEXT("Weight_FirstPerson"));


AALSPlayerCameraManager::AALSPlayerCameraManager()
{
	CameraBehavior = CreateDefaultSubobject<USkeletalMeshComponent>(NAME_CameraBehavior);
	CameraBehavior->SetupAttachment(GetRootComponent());
	CameraBehavior->bHiddenInGame = true;

	CurrentCameraDistance = TargetCameraDistance = 650.0f;
}

void AALSPlayerCameraManager::OnPossess(AALSBaseCharacter* NewCharacter)
{
	// Set "Controlled Pawn" when Player Controller Possesses new character. (called from Player Controller)
	check(NewCharacter);
	ControlledCharacter = NewCharacter;

	// Update references in the Camera Behavior AnimBP.
	UALSPlayerCameraBehavior* CastedBehv = Cast<UALSPlayerCameraBehavior>(CameraBehavior->GetAnimInstance());
	if (CastedBehv)
	{
		NewCharacter->SetCameraBehavior(CastedBehv);
		CastedBehv->MovementState = NewCharacter->GetMovementState();
		CastedBehv->MovementAction = NewCharacter->GetMovementAction();
		CastedBehv->bRightShoulder = NewCharacter->IsRightShoulder();
		CastedBehv->Gait = NewCharacter->GetGait();
		CastedBehv->SetRotationMode(NewCharacter->GetRotationMode());
		CastedBehv->Stance = NewCharacter->GetStance();
		CastedBehv->ViewMode = NewCharacter->GetViewMode();
	}

	// Initial position
	const FVector& TPSLoc = ControlledCharacter->GetThirdPersonPivotTarget().GetLocation();
	SetActorLocation(TPSLoc);
	SmoothedPivotTarget.SetLocation(TPSLoc);

	ALSDebugComponent = ControlledCharacter->FindComponentByClass<UALSDebugComponent>();
}

float AALSPlayerCameraManager::GetCameraBehaviorParam(FName CurveName) const
{
	UAnimInstance* Inst = CameraBehavior->GetAnimInstance();
	if (Inst)
	{
		return Inst->GetCurveValue(CurveName);
	}
	return 0.0f;
}

void AALSPlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	// Partially taken from base class

	if (OutVT.Target)
	{
		FVector OutLocation;
		FRotator OutRotation;
		float OutFOV;

		if (OutVT.Target->IsA<AALSBaseCharacter>())
		{
			if (CustomCameraBehavior(DeltaTime, OutLocation, OutRotation, OutFOV))
			{
				OutVT.POV.Location = OutLocation;
				OutVT.POV.Rotation = OutRotation;
				OutVT.POV.FOV = OutFOV;
			}
			else
			{
				OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
			}
		}
		else
		{
			OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
		}
	}
}

FVector AALSPlayerCameraManager::CalculateAxisIndependentLag(FVector CurrentLocation, FVector TargetLocation,
                                                             FRotator CameraRotation, FVector LagSpeeds,
                                                             float DeltaTime)
{
	CameraRotation.Roll = 0.0f;
	CameraRotation.Pitch = 0.0f;
	const FVector UnrotatedCurLoc = CameraRotation.UnrotateVector(CurrentLocation);
	const FVector UnrotatedTargetLoc = CameraRotation.UnrotateVector(TargetLocation);

	const FVector ResultVector(
		FMath::FInterpTo(UnrotatedCurLoc.X, UnrotatedTargetLoc.X, DeltaTime, LagSpeeds.X),
		FMath::FInterpTo(UnrotatedCurLoc.Y, UnrotatedTargetLoc.Y, DeltaTime, LagSpeeds.Y),
		FMath::FInterpTo(UnrotatedCurLoc.Z, UnrotatedTargetLoc.Z, DeltaTime, LagSpeeds.Z));

	return CameraRotation.RotateVector(ResultVector);
}

void AALSPlayerCameraManager::AddCameraZoom(float Value)
{
	// Mouse wheel usually gives small positive/negative values. Positive = scroll up.
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	// Subtract so scrolling up zooms in
	TargetCameraDistance = FMath::Clamp(
		TargetCameraDistance - Value * CameraZoomStep,
		MinCameraDistance,
		MaxCameraDistance);
}

bool AALSPlayerCameraManager::CustomCameraBehavior(float DeltaTime, FVector& Location, FRotator& Rotation, float& FOV)
{
	if (!ControlledCharacter)
	{
		return false;
	}

	// --- STEP 1: Get Camera Params from Character (still uses ALS interface) ---

	const FTransform& PivotTarget = ControlledCharacter->GetThirdPersonPivotTarget();
	const FVector& FPTarget = ControlledCharacter->GetFirstPersonCameraTarget();

	// We treat this as "third person FOV" for MMO
	float TPFOV = 80.0f; // Slightly narrower than 90, feels more MMO-ish
	float FPFOV = 80.0f;
	bool bRightShoulder = false;
	ControlledCharacter->GetCameraParameters(TPFOV, FPFOV, bRightShoulder);

	// --- STEP 2: Target Camera Rotation from Control Rotation (with lag + pitch clamp) ---

	const APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return false;
	}

	const FRotator ControlRot = PC->GetControlRotation();
	const float RotationLagSpeed = GetCameraBehaviorParam(NAME_RotationLagSpeed);

	// Smooth towards control rotation
	const FRotator InterpResult = FMath::RInterpTo(
		GetCameraRotation(),
		ControlRot,
		DeltaTime,
		RotationLagSpeed);

	// Keep ALS debug override behaviour
	TargetCameraRotation = UKismetMathLibrary::RLerp(
		InterpResult,
		DebugViewRotation,
		GetCameraBehaviorParam(NAME_Override_Debug),
		true);

	// MMO-style pitch clamp so you can't flip around the character
	TargetCameraRotation.Pitch = FMath::Clamp(TargetCameraRotation.Pitch, -60.0f, 20.0f);
	TargetCameraRotation.Roll = 0.0f;

	// --- STEP 3: Smoothed Pivot Target (same ALS axis-independent lag) ---

	const FVector LagSpd(
		GetCameraBehaviorParam(NAME_PivotLagSpeed_X),
		GetCameraBehaviorParam(NAME_PivotLagSpeed_Y),
		GetCameraBehaviorParam(NAME_PivotLagSpeed_Z));

	const FVector AxisIndpLag = CalculateAxisIndependentLag(
		SmoothedPivotTarget.GetLocation(),
		PivotTarget.GetLocation(),
		TargetCameraRotation,
		LagSpd,
		DeltaTime);

	SmoothedPivotTarget.SetLocation(AxisIndpLag);
	SmoothedPivotTarget.SetRotation(PivotTarget.GetRotation());
	SmoothedPivotTarget.SetScale3D(FVector::OneVector);

	// --- STEP 4: MMO-style fixed offsets + smooth zoom ---

	// Tunable feel knobs:
	const float MMO_PivotOffsetZ = 70.0f;   // how high above the character the pivot is
	const float MMO_CameraHeight = 130.0f;  // extra height above pivot
	const float MMO_CameraSideOffset = 40.0f;   // small right-shoulder offset; set to 0 for dead-center

	// Smooth camera zoom (Current → Target)
	CurrentCameraDistance = FMath::FInterpTo(
		CurrentCameraDistance,
		TargetCameraDistance,
		DeltaTime,
		CameraZoomInterpSpeed);

	// Pivot roughly around the upper body
	PivotLocation =
		SmoothedPivotTarget.GetLocation() +
		UKismetMathLibrary::GetUpVector(SmoothedPivotTarget.Rotator()) * MMO_PivotOffsetZ;

	// MMO camera: behind the character, slightly above and to the right
	TargetCameraLocation =
		PivotLocation +
		UKismetMathLibrary::GetForwardVector(TargetCameraRotation) * -CurrentCameraDistance +
		UKismetMathLibrary::GetRightVector(TargetCameraRotation) * MMO_CameraSideOffset +
		UKismetMathLibrary::GetUpVector(TargetCameraRotation) * MMO_CameraHeight;

	// --- STEP 5: Obstacle avoidance (unchanged ALS sphere trace logic) ---

	FVector TraceOrigin;
	float TraceRadius;
	ECollisionChannel TraceChannel = ControlledCharacter->GetThirdPersonTraceParams(TraceOrigin, TraceRadius);

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(ControlledCharacter);

	FHitResult HitResult;
	const FCollisionShape SphereCollisionShape = FCollisionShape::MakeSphere(TraceRadius);

	const bool bHit = World->SweepSingleByChannel(
		HitResult,
		TraceOrigin,
		TargetCameraLocation,
		FQuat::Identity,
		TraceChannel,
		SphereCollisionShape,
		Params);

	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugSphereTraceSingle(
			World,
			TraceOrigin,
			TargetCameraLocation,
			SphereCollisionShape,
			EDrawDebugTrace::Type::ForOneFrame,
			bHit,
			HitResult,
			FLinearColor::Red,
			FLinearColor::Green,
			5.0f);
	}

	if (HitResult.IsValidBlockingHit())
	{
		TargetCameraLocation += HitResult.Location - HitResult.TraceEnd;
	}

	// --- STEP 6: Final transform (always 3rd person, keep debug override) ---

	FTransform TargetCameraTransform(TargetCameraRotation, TargetCameraLocation, FVector::OneVector);
	FTransform FPTargetCameraTransform(TargetCameraRotation, FPTarget, FVector::OneVector);

	// MMO: no first-person blend – we lock this to pure third person
	const float FirstPersonWeight = 0.0f;

	const FTransform MixedTransform = UKismetMathLibrary::TLerp(
		TargetCameraTransform,
		FPTargetCameraTransform,
		FirstPersonWeight);

	const FTransform TargetTransform = UKismetMathLibrary::TLerp(
		MixedTransform,
		FTransform(DebugViewRotation, TargetCameraLocation, FVector::OneVector),
		GetCameraBehaviorParam(NAME_Override_Debug));

	Location = TargetTransform.GetLocation();
	Rotation = TargetTransform.Rotator();

	// Pure third-person FOV for MMO
	FOV = FMath::Lerp(TPFOV, FPFOV, FirstPersonWeight);

	return true;
}


