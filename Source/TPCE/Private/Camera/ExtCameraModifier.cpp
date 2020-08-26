// Copyright (c) 2020 greisane <ggreisane@gmail.com>

#include "Camera/ExtCameraModifier.h"

#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "ExtraMacros.h"

UExtCameraModifier::UExtCameraModifier()
	: bPauseWithPlayerInput(false)
	, UnpauseAfter(3.0f)
	, AlphaSmoothingExp(2.0f)
{
}

void UExtCameraModifier::AddedToCamera(APlayerCameraManager* Camera)
{
	Super::AddedToCamera(Camera);

	BlueprintAddedToCamera(Camera);

	bPaused = false;
	TimePaused = 0.0f;
	Alpha = SmoothAlpha = 0.0f;
}

bool UExtCameraModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	// Full override is needed to fix what seems like an engine bug
	// Docs specify that camera modifier alpha is taken into account, however no blending actually happens
	FULL_OVERRIDE();

	// Update the alpha
	UpdateAlpha(DeltaTime);

	// Blend camera state
	FVector NewPOVLocation = InOutPOV.Location;
	FRotator NewPOVRotation = InOutPOV.Rotation;
	float NewPOVFOV = InOutPOV.FOV;

	BlueprintModifyCamera(DeltaTime, NewPOVLocation, NewPOVRotation, NewPOVFOV, NewPOVLocation, NewPOVRotation, NewPOVFOV);
	const bool bResult = NativeModifyCamera(DeltaTime, NewPOVLocation, NewPOVRotation, NewPOVFOV);

	InOutPOV.Location = FMath::Lerp(InOutPOV.Location, NewPOVLocation, SmoothAlpha);
	const FRotator DeltaAng = (NewPOVRotation - InOutPOV.Rotation).GetNormalized();
	InOutPOV.Rotation = InOutPOV.Rotation + SmoothAlpha * DeltaAng;
	InOutPOV.FOV = FMath::Lerp(InOutPOV.FOV, NewPOVFOV, SmoothAlpha);

	if (CameraOwner)
	{
		// Pushing these through the cached PP blend system in the camera to get proper layered blending,
		// rather than letting subsequent mods stomp over each other in the InOutPOV struct.
		float PPBlendWeight = 0.0f;
		FPostProcessSettings PPSettings;
		BlueprintModifyPostProcess(DeltaTime, PPBlendWeight, PPSettings);

		if (PPBlendWeight > 0.0f)
		{
			CameraOwner->AddCachedPPBlend(PPSettings, PPBlendWeight);
		}
	}

	// If pending disable and fully alpha'd out, truly disable this modifier
	if (bPendingDisable && Alpha <= 0.0f)
	{
		DisableModifier(true);
	}

	return bResult;
}

void UExtCameraModifier::UpdateAlpha(float DeltaTime)
{
	Super::UpdateAlpha(DeltaTime);

	SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, AlphaSmoothingExp);
}

bool UExtCameraModifier::ProcessViewRotation(AActor* ViewTarget, float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
	TimePaused += DeltaTime;
	if (bPauseWithPlayerInput && !OutDeltaRot.IsZero())
	{
		bPaused = true;
		TimePaused = 0.0f;
	}
	else if (bPaused && UnpauseAfter > 0.0f && TimePaused >= UnpauseAfter)
	{
		bPaused = false;
	}

	FRotator NewViewRotation = OutViewRotation;

	// Let BP do what it wants
	BlueprintProcessViewRotation(ViewTarget, DeltaTime, OutDeltaRot, NewViewRotation, NewViewRotation);
	const bool bResult = NativeProcessViewRotation(ViewTarget, DeltaTime, OutDeltaRot, NewViewRotation);

	const FRotator DeltaViewRotation = (NewViewRotation - OutViewRotation).GetNormalized();
	OutViewRotation = OutViewRotation + SmoothAlpha * DeltaViewRotation;

	return bResult;
}

float UExtCameraModifier::GetTargetAlpha()
{
	return (bPendingDisable || bPaused) ? 0.0f : 1.0f;
}

bool UExtCameraModifier::NativeModifyCamera(float DeltaTime, FVector& ViewLocation, FRotator& ViewRotation, float& FOV)
{
	// Do nothing in base class and allow subsequent modifiers to update
	return false;
}

bool UExtCameraModifier::NativeProcessViewRotation(AActor* ViewTarget, float DeltaTime, const FRotator& DeltaRot, FRotator& OutViewRotation)
{
	// Do nothing in base class and allow subsequent modifiers to update
	return false;
}

bool UExtCameraModifier::IsPaused() const
{
	return bPaused;
}

APawn* UExtCameraModifier::GetPawn() const
{
	if (CameraOwner)
	{
		if (APlayerController* PC = CameraOwner->GetOwningPlayerController())
		{
			return PC->GetPawn();
		}
	}
	return nullptr;
}
