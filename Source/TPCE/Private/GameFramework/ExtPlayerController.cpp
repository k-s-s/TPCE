// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "GameFramework/ExtPlayerController.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/ExtPlayerCameraManager.h"
#include "Components/ExtInputComponent.h"
#include "Interfaces/PawnControlInterface.h"

#include "Logging/LogMacros.h"
#include "Kismet/Kismet.h"

#include "TPCEMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogExtPlayerController, Log, All);

#define LOCTEXT_NAMESPACE "ExtPlayerController"

AExtPlayerController::AExtPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AExtPlayerCameraManager::StaticClass();
	bAutoManageActiveCameraTarget = true;	// Auto set view target when a pawn is possessed/unpossessed

	AutoManagedCameraTransitionParams.BlendTime = 1.0f;
	AutoManagedCameraTransitionParams.BlendFunction = EViewTargetBlendFunction::VTBlend_Cubic;
	AutoManagedCameraTransitionParams.BlendExp = 2.0f;
}

void AExtPlayerController::SetupInputComponent()
{
	if (InputComponent == NULL)
	{
		InputComponent = NewObject<UExtInputComponent>(this, TEXT("PC_InputComponent0"));
		InputComponent->RegisterComponent();
	}

	Super::SetupInputComponent();
}

void AExtPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bBeganPlay = true;
}

AActor* AExtPlayerController::GetViewTarget() const
{
	// If fading to another view target, return that one first
	if (PendingFadeViewTarget)
	{
		return PendingFadeViewTarget;
	}

	return Super::GetViewTarget();
}

void AExtPlayerController::SetViewTarget(class AActor* NewViewTarget, struct FViewTargetTransitionParams TransitionParams)
{
	// Possessing a pawn will first set the view target to the player controller
	// When auto managed camera is set to fade to black we will want to undo this change
	OldViewTarget = Super::GetViewTarget();

	Super::SetViewTarget(NewViewTarget, TransitionParams);
}

void AExtPlayerController::AutoManageActiveCameraTarget(AActor* SuggestedTarget)
{
	// Full override to support smooth transitions when changing view target
	FULL_OVERRIDE();

	if (bAutoManageActiveCameraTarget)
	{
		// See if there is a CameraActor with an auto-activate index that matches us.
		if (GetNetMode() == NM_Client)
		{
			// Clients don't know their own index on the server, so they have to trust that if they use a camera with an auto-activate index, that's their own index.
			ACameraActor* CurrentCameraActor = Cast<ACameraActor>(GetViewTarget());
			if (CurrentCameraActor)
			{
				const int32 CameraAutoIndex = CurrentCameraActor->GetAutoActivatePlayerIndex();
				if (CameraAutoIndex != INDEX_NONE)
				{
					return;
				}
			}
		}
		else
		{
			// See if there is a CameraActor in the level that auto-activates for this PC.
			ACameraActor* AutoCameraTarget = GetAutoActivateCameraForPlayer();
			if (AutoCameraTarget && AutoCameraTarget != GetViewTarget())
			{
				if (!bBeganPlay)
				{
					// Don't blend into the view target when entering playing state
					SetViewTarget(AutoCameraTarget);
				}
				else if (AutoManagedCameraTransitionParams.bFadeToBlack)
				{
					if (OldViewTarget != this)
					{
						SetViewTarget(OldViewTarget);
					}
					SetViewTargetWithFade(AutoCameraTarget, AutoManagedCameraTransitionParams.BlendTime, FLinearColor::Black, AutoManagedCameraTransitionParams.bShouldFadeAudio);
				}
				else
				{
					SetViewTarget(AutoCameraTarget, AutoManagedCameraTransitionParams);
				}
				return;
			}
		}

		// No auto-activate CameraActor, so use the suggested target.
		if (SuggestedTarget != GetViewTarget())
		{
			if (!bBeganPlay)
			{
				// Don't blend into the view target when entering playing state
				SetViewTarget(SuggestedTarget);
			}
			else if (AutoManagedCameraTransitionParams.bFadeToBlack)
			{
				if (OldViewTarget != this)
				{
					SetViewTarget(OldViewTarget);
				}
				SetViewTargetWithFade(SuggestedTarget, AutoManagedCameraTransitionParams.BlendTime, FLinearColor::Black, AutoManagedCameraTransitionParams.bShouldFadeAudio);
			}
			else
			{
				SetViewTarget(SuggestedTarget, AutoManagedCameraTransitionParams);
			}
		}
	}
}

void AExtPlayerController::PushInputComponent(UInputComponent* InInputComponent)
{
	Super::PushInputComponent(InInputComponent);

	if (InInputComponent)
	{
		OnInputStackChanged.Broadcast();
	}
}

bool AExtPlayerController::PopInputComponent(UInputComponent* InInputComponent)
{
	if (Super::PopInputComponent(InInputComponent))
	{
		OnInputStackChanged.Broadcast();
		return true;
	}
	return false;
}

EPlayerControllerInputDevices AExtPlayerController::GetKeyInputDevices(FKey Key)
{
	if (Key.IsGamepadKey())
	{
		return EPlayerControllerInputDevices::Gamepad;
	}
	else if (Key.IsMouseButton())
	{
		return EPlayerControllerInputDevices::Mouse;
	}
	else if (Key.IsTouch())
	{
		return EPlayerControllerInputDevices::Touch;
	}
	else
	{
		return EPlayerControllerInputDevices::Keyboard;
	}
}

void AExtPlayerController::GetInputBindingDescriptions(TArray<FInputBindingDescription>& OutInputBindingDescriptions)
{
	OutInputBindingDescriptions.Empty();

	TArray<UInputComponent*> InputComponentStack;
	BuildInputStack(InputComponentStack);

	// Walk the stack, top to bottom
	int32 StackIndex = InputComponentStack.Num() - 1;
	for (; StackIndex >= 0; --StackIndex)
	{
		if (const UExtInputComponent* ExtInputComponent = Cast<UExtInputComponent>(InputComponentStack[StackIndex]))
		{
			// Go through bindings and get the key mappings that have an user provided description
			const int32 NumActionBindings = ExtInputComponent->GetNumActionBindings();
			for (int32 ActionBindingIndex = 0; ActionBindingIndex < NumActionBindings; ActionBindingIndex++)
			{
				FText Text;
				const FInputActionBinding& ActionBinding = ExtInputComponent->GetActionBinding(ActionBindingIndex);
				if (ExtInputComponent->GetActionBindingDescriptionForHandle(ActionBinding.GetHandle(), Text))
				{
					const TArray<FInputActionKeyMapping>& KeysForAction = PlayerInput->GetKeysForAction(ActionBinding.GetActionName());
					for (const FInputActionKeyMapping& KeyMapping : KeysForAction)
					{
						if (EnumHasAnyFlags(GetKeyInputDevices(KeyMapping.Key), (EPlayerControllerInputDevices)InputDevices))
						{
							// Find an element with the same description or add a new one
							FInputBindingDescription* InputBindingDescription = OutInputBindingDescriptions.FindByPredicate([&](FInputBindingDescription El)
								{
									return El.Text.EqualTo(Text);
								});
							if (!InputBindingDescription)
							{
								InputBindingDescription = &OutInputBindingDescriptions.Emplace_GetRef();
							}

							InputBindingDescription->Text = Text;
							InputBindingDescription->KeyMappings.Add(KeyMapping);
						}
					}
				}
			}
		}

		if (InputComponentStack[StackIndex]->bBlockInput)
		{
			// Stop traversing the stack, all input has been consumed by this InputComponent
			break;
		}
	}
}

bool AExtPlayerController::DeprojectMousePositionToPlane(FVector& WorldLocation, const FPlane& Plane = FPlane(FVector::UpVector, 0.0f)) const
{
	FVector Location, Direction;

	if (DeprojectMousePositionToWorld(Location, Direction))
	{
		float T; // ignored
		return Kismet::Math::RayPlaneIntersection(Location, Direction, Plane, T, WorldLocation);
	}

	return false;
}

bool AExtPlayerController::DeprojectMousePositionToGround(FVector& WorldLocation) const
{
	return DeprojectMousePositionToPlane(WorldLocation, GetGroundPlane());
}

bool AExtPlayerController::DeprojectScreenPositionToPlane(float ScreenX, float ScreenY, FVector& WorldLocation, const FPlane& Plane = FPlane(FVector::UpVector, 0.0f)) const
{
	FVector Location, Direction;

	if (DeprojectScreenPositionToWorld(ScreenX, ScreenY, Location, Direction))
	{
		float T; // ignored
		return Kismet::Math::RayPlaneIntersection(Location, Direction, Plane, T, WorldLocation);
	}

	return false;
}

bool AExtPlayerController::DeprojectScreenPositionToGround(float ScreenX, float ScreenY, FVector& WorldLocation) const
{
	return DeprojectScreenPositionToPlane(ScreenX, ScreenY, WorldLocation, GetGroundPlane());
}

FPlane AExtPlayerController::GetGroundPlane() const
{
	if (APawn* MyPawn = GetPawn())
	{
		return FPlane(MyPawn->GetActorLocation() - FVector(0.f, 0.f, MyPawn->GetDefaultHalfHeight()), FVector::UpVector);
	}

	return FPlane(RootComponent->GetComponentLocation(), FVector::UpVector);
}

void AExtPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (IPawnControlInterface* PawnControl = Cast<IPawnControlInterface>(GetPawnOrSpectator()))
	{
		PawnControl->ProcessInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AExtPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	UpdateViewExtents();

	if (PendingFadeViewTarget && PlayerCameraManager && PlayerCameraManager->FadeTimeRemaining <= 0.f)
	{
		// Camera fade from SetViewTargetWithFade complete. First allow blueprint to stop the transition
		ReceiveFadeToBlack(PendingFadeViewTarget);

		if (PendingFadeViewTarget)
		{
			// Switch to new view target and fade back in
			PlayerCameraManager->SetViewTarget(PendingFadeViewTarget);
			PlayerCameraManager->StartCameraFade(1.f, 0.f, PlayerCameraManager->FadeTime, PlayerCameraManager->FadeColor, PlayerCameraManager->bFadeAudio, false);
			PendingFadeViewTarget = nullptr;
		}
	}

#if ENABLE_DRAW_DEBUG
	if (bDebugCamera && bViewExtentsValid)
	{
		if (UWorld* World = GetWorld())
		{
			const float LineThickness = 2.f;
			DrawDebugLine(World, ViewCorners[0], ViewCorners[1], FColor::Yellow, false, 0.f, SDPG_Foreground, LineThickness);
			DrawDebugLine(World, ViewCorners[1], ViewCorners[2], FColor::Yellow, false, 0.f, SDPG_Foreground, LineThickness);
			DrawDebugLine(World, ViewCorners[2], ViewCorners[3], FColor::Yellow, false, 0.f, SDPG_Foreground, LineThickness);
			DrawDebugLine(World, ViewCorners[3], ViewCorners[0], FColor::Yellow, false, 0.f, SDPG_Foreground, LineThickness);
		}
	}
#endif
}

bool AExtPlayerController::GetViewExtents(FVector& TopLeft, FVector& TopRight, FVector& BottomRight, FVector& BottomLeft, FVector& Min, FVector& Max) const
{
	if (!bViewExtentsValid)
	{
		TopLeft = TopRight = BottomRight = BottomLeft = Min = Max = FVector::ZeroVector;
		return false;
	}

	TopLeft = ViewCorners[0];
	TopRight = ViewCorners[1];
	BottomRight = ViewCorners[2];
	BottomLeft = ViewCorners[3];
	Min = ViewExtentsMin;
	Max = ViewExtentsMax;
	return true;
}

void AExtPlayerController::UpdateViewExtents()
{
	bViewExtentsValid = false;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer && LocalPlayer->ViewportClient && LocalPlayer->ViewportClient->Viewport)
	{
		// Create a view family for the game viewport
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			LocalPlayer->ViewportClient->Viewport,
			GetWorld()->Scene,
			LocalPlayer->ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true));

		// Calculate player view
		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);

		if (SceneView)
		{
			const FIntRect ViewRect = SceneView->UnconstrainedViewRect;
			const FMatrix ViewProjInvMatrix = SceneView->ViewMatrices.GetInvViewProjectionMatrix();
			const FPlane GroundPlane = GetGroundPlane();

			auto DeprojectScreenPositionToPlane = [&](float ScreenX, float ScreenY, FVector& WorldLocation) -> bool
			{
				const FVector2D ScreenXY = FVector2D(ScreenX, ScreenY);
				FVector Location, Direction;
				FSceneView::DeprojectScreenToWorld(ScreenXY, ViewRect, ViewProjInvMatrix, /*out*/ Location, /*out*/ Direction);
				float T;  // Ignored
				return UKismetMathLibraryEx::RayPlaneIntersection(Location, Direction, GroundPlane, T, WorldLocation);
			};

			bViewExtentsValid = true;
			bViewExtentsValid = bViewExtentsValid && DeprojectScreenPositionToPlane(ViewRect.Min.X, ViewRect.Min.Y, /*out*/ ViewCorners[0]);
			bViewExtentsValid = bViewExtentsValid && DeprojectScreenPositionToPlane(ViewRect.Max.X, ViewRect.Min.Y, /*out*/ ViewCorners[1]);
			bViewExtentsValid = bViewExtentsValid && DeprojectScreenPositionToPlane(ViewRect.Max.X, ViewRect.Max.Y, /*out*/ ViewCorners[2]);
			bViewExtentsValid = bViewExtentsValid && DeprojectScreenPositionToPlane(ViewRect.Min.X, ViewRect.Max.Y, /*out*/ ViewCorners[3]);

			ViewExtentsMin = ViewCorners[0];
			ViewExtentsMax = ViewCorners[0];
			for (int32 ViewCornerIdx = 1; ViewCornerIdx < 4; ViewCornerIdx++)
			{
				ViewExtentsMin = ViewExtentsMin.ComponentMin(ViewCorners[ViewCornerIdx]);
				ViewExtentsMax = ViewExtentsMax.ComponentMax(ViewCorners[ViewCornerIdx]);
			}
		}
	}
}

void AExtPlayerController::SetViewTargetWithFade(AActor* NewViewTarget, float BlendTime, FLinearColor Color, bool bShouldFadeAudio)
{
	if (NewViewTarget == nullptr)
	{
		// Make sure view target is valid
		NewViewTarget = this;
	}

	if (PlayerCameraManager)
	{
		if (!PendingFadeViewTarget)
		{
			PlayerCameraManager->StartCameraFade(0.f, 1.f, BlendTime * .5f, Color, bShouldFadeAudio, true);
		}

		PendingFadeViewTarget = NewViewTarget;
	}
}

#undef LOCTEXT_NAMESPACE
