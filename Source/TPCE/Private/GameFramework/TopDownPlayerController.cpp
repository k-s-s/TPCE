// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "GameFramework/TopDownPlayerController.h"

#include "GameFramework/Pawn.h"
#include "Components/ArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/TopDownPushToTargetComponent.h"
#include "Components/InputComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Engine/CollisionProfile.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/KismetMathLibraryExtensions.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogTopDownPlayerController, Log, All);

ATopDownPlayerController::ATopDownPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShowMouseCursor = true;

	// Create camera mout
	static const FName CameraMountName(TEXT("CameraMount"));
	CameraMount = CreateDefaultSubobject<USphereComponent>(CameraMountName);
	CameraMount->SetupAttachment(RootComponent);
	CameraMount->SetGenerateOverlapEvents(false);
	CameraMount->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	static const FName CameraMountCollisionProfileName(TEXT("CameraActor"));
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	const UCollisionProfile& CollisionProfiles = *UCollisionProfile::Get();
	const int32 ProfilesCount = CollisionProfiles.GetNumOfProfiles();
	bool bIsProfileValid = false;
	for (int32 i = 0; !bIsProfileValid && i < ProfilesCount; ++i)
		bIsProfileValid = CollisionProfiles.GetProfileByIndex(i)->Name == CameraMountCollisionProfileName;
	if (!bIsProfileValid)
		UE_LOG(LogTopDownPlayerController, Warning, TEXT("Collision profile not found: '%s'. Camera mount created with default collision settings."), *CameraMountCollisionProfileName.ToString());
#endif

	CameraMount->SetCollisionProfileName(CameraMountCollisionProfileName);
	CameraMount->SetCanEverAffectNavigation(false);
	CameraMount->SetAbsolute(true, true, true);
	CameraMount->SetIsReplicated(false);

	// Create camera boom
	static const FName CameraBoomName(TEXT("CameraBoom"));
	CameraBoom = CreateDefaultSubobject<UArmComponent>(CameraBoomName);
	CameraBoom->SetupAttachment(CameraMount);
	CameraBoom->SetWorldRotation(FRotator(-53.1301024f, -45.0f, 0.0f));
	CameraBoom->TargetArmLength = 2200.0f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bEnableTargetLag = true;
	CameraBoom->TargetLagSpeed = 10.0f;
	CameraBoom->bUseTargetLagSubstepping = true;
	CameraBoom->TargetLagMaxTimeStep = 1.f / 30.f;
	CameraBoom->SetIsReplicated(false);

	// Create camera
	static const FName CameraComponentName(TEXT("CameraComponent"));
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(CameraComponentName);
	CameraComponent->SetupAttachment(CameraBoom, UArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	CameraComponent->FieldOfView = 60.0f;
	CameraComponent->SetIsReplicated(false);

	// Create camera tractor
	static const FName CameraTractorName(TEXT("CameraTractor"));
	CameraTractor = CreateDefaultSubobject<UTopDownPushToTargetComponent>(CameraTractorName);
	CameraTractor->bEnableLag = true;
	CameraTractor->Speed = 1.0f;
	CameraTractor->bTeleportToTargetToStart = true;
	CameraTractor->bSlide = true;
	CameraTractor->Friction = 0.0f;
	CameraTractor->bForceSubStepping = true;
	CameraTractor->MaxSimulationTimeStep = 1.f / 30.f;
	CameraTractor->bAdjustTargetLagForViewTarget = true;
	CameraTractor->SetUpdatedComponent(CameraMount);
	CameraTractor->SetIsReplicated(false);
}

void ATopDownPlayerController::ForceAbsolute(USceneComponent* SceneComponent)
{
	check(SceneComponent);
	bool bAbsolute = SceneComponent->IsUsingAbsoluteLocation() && SceneComponent->IsUsingAbsoluteRotation() && SceneComponent->IsUsingAbsoluteScale();
	if (!bAbsolute)
	{
		UE_LOG(LogTopDownPlayerController, Warning, TEXT("'%s' coordinates need to be defined in world-space and were changed to absolute."), *SceneComponent->GetReadableName());
		SceneComponent->SetAbsolute(true, true, true);
	}
}

void ATopDownPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ForceAbsolute(CameraMount);

	// Some Physics related methods require the GEngine global to be created and cannot be on the constructor.
	CameraMount->SetMassOverrideInKg(NAME_None, 1.0f);
	CameraMount->SetLinearDamping(0.1f);
	CameraMount->SetEnableGravity(false);
	CameraMount->bIgnoreRadialImpulse = true;
	CameraMount->bIgnoreRadialImpulse = true;
	CameraMount->SetShouldUpdatePhysicsVolume(false);
}

void ATopDownPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CameraMount->SetWorldLocation(RootComponent->GetComponentLocation());
}

void ATopDownPlayerController::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	check(CameraComponent);
	CameraComponent->GetCameraView(DeltaTime, OutResult);
}

bool ATopDownPlayerController::HasActiveCameraComponent() const
{
	check(CameraComponent);
	return CameraComponent->IsActive();
}

bool ATopDownPlayerController::HasActivePawnControlCameraComponent() const
{
	check(CameraComponent);
	return (CameraComponent->IsActive() && CameraComponent->bUsePawnControlRotation);
}

void ATopDownPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	UpdateViewExtents();

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

void ATopDownPlayerController::SetCameraTargetComponent(USceneComponent* NewTargetComponent, const FName& SocketName)
{
	if (CameraTractor)
		CameraTractor->SetTargetComponent(NewTargetComponent, SocketName);
}

bool ATopDownPlayerController::GetViewExtents(FVector& TopLeft, FVector& TopRight, FVector& BottomRight, FVector& BottomLeft, FVector& Min, FVector& Max) const
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

void ATopDownPlayerController::UpdateViewExtents()
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
			const FPlane GroundPlane(CameraMount->GetComponentLocation(), FVector::UpVector);

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
