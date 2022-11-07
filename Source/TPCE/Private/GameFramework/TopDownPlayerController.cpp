// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "GameFramework/TopDownPlayerController.h"

#include "Camera/CameraComponent.h"
#include "Components/ArmComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/TopDownPushToTargetComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Canvas.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogTopDownPlayerController, Log, All);

ATopDownPlayerController::ATopDownPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bGroundOverlay(false)
	, OverlayZ(5.f)
	, OverlayHeight(20.f)
	, OverlayCanvasSize(1024)
	, OverlayPixelsPerUnit(1.f)
{
	// Player controllers are normally invisible, but this unnecessarily complicates things
	// If it becomes invisible again for some reason, check that ADebugCameraController::OnDeactivate isn't at fault
	SetHidden(false);
#if WITH_EDITORONLY_DATA
	bHiddenEd = false;
#endif // WITH_EDITORONLY_DATA
	bShowMouseCursor = true;

	// Create camera mount
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
	{
		bIsProfileValid = CollisionProfiles.GetProfileByIndex(i)->Name == CameraMountCollisionProfileName;
	}
	if (!bIsProfileValid)
	{
		UE_LOG(LogTopDownPlayerController, Warning, TEXT("Collision profile not found: '%s'. Camera mount created with default collision settings."), *CameraMountCollisionProfileName.ToString());
	}
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

	// Create overlay decal
	static const FName GroundOverlayName(TEXT("GroundOverlay"));
	static ConstructorHelpers::FObjectFinder<UMaterial> OverlayMaterialObject(TEXT("Material'/TPCE/M_TopDownPlayerControllerOverlay.M_TopDownPlayerControllerOverlay'"));
	GroundOverlay = CreateDefaultSubobject<UDecalComponent>(GroundOverlayName);
	GroundOverlay->SetupAttachment(GetRootComponent());
	GroundOverlay->SetAbsolute(true, true, true);
	GroundOverlay->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	GroundOverlay->SetDecalMaterial(OverlayMaterialObject.Object);
	GroundOverlay->SetVisibility(false);
	GroundOverlay->bDestroyOwnerAfterFade = false;
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

	// Create overlay render target texture to be drawn to
	OverlayCanvasRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), 512, 512);
	if (OverlayCanvasRenderTarget)
	{
		OverlayCanvasRenderTarget->ClearColor = FLinearColor::Transparent;
		OverlayCanvasRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &ATopDownPlayerController::DrawGroundOverlay);

		if (UMaterialInstanceDynamic* OverlayMID = GroundOverlay->CreateDynamicMaterialInstance())
		{
			static const FName OverlayMaterialTextureParameterName(TEXT("RenderTarget"));
			OverlayMID->SetTextureParameterValue(OverlayMaterialTextureParameterName, OverlayCanvasRenderTarget);
		}
	}
}

void ATopDownPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	UpdateGroundOverlay();
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

FPlane ATopDownPlayerController::GetGroundPlane() const
{
	if (GetPawn())
	{
		return Super::GetGroundPlane();
	}

	return FPlane(CameraMount->GetComponentLocation(), FVector::UpVector);
}

void ATopDownPlayerController::SetCameraTargetComponent(USceneComponent* NewTargetComponent, const FName& SocketName)
{
	if (CameraTractor)
	{
		CameraTractor->SetTargetComponent(NewTargetComponent, SocketName);
	}
}

void ATopDownPlayerController::UpdateGroundOverlay()
{
	bool bShowOverlay = false;

	if (bGroundOverlay && bViewExtentsValid)
	{
		FVector CanvasSize = (ViewExtentsMax - ViewExtentsMin) * OverlayPixelsPerUnit;
		FVector CanvasScale = FVector(1.f / OverlayPixelsPerUnit, 1.f / OverlayPixelsPerUnit, 1.f);
		if (CanvasSize.X != OverlayCanvasSize)
		{
			CanvasScale.X /= OverlayCanvasSize / CanvasSize.X;
			CanvasSize.X = OverlayCanvasSize;
		}
		if (CanvasSize.Y != OverlayCanvasSize)
		{
			CanvasScale.Y /= OverlayCanvasSize / CanvasSize.Y;
			CanvasSize.Y = OverlayCanvasSize;
		}

		FVector OverlayOrigin = (ViewExtentsMin + ViewExtentsMax) * .5f;
		OverlayOrigin.Z = OverlayZ;
		const FVector CanvasOrigin = OverlayOrigin * FVector(1.f, 1.f, 0.f) - (CanvasSize * CanvasScale * .5f);
		OverlayCanvasTransform = FScaleRotationTranslationMatrix(CanvasScale, FRotator::ZeroRotator, CanvasOrigin).Inverse();

		// Reposition decal
		const FTransform DecalTransform = FTransform(FRotator(-90.f, 0.f, 0.f), OverlayOrigin, FVector(1.f, CanvasScale.Y, CanvasScale.X));
		GroundOverlay->SetWorldTransform(DecalTransform);
		GroundOverlay->DecalSize = FVector(OverlayHeight, CanvasSize.Y * .5f, CanvasSize.X * .5f);

		if (CanvasSize.X > 0 && CanvasSize.Y > 0 && OverlayCanvasRenderTarget)
		{
			// Finally render if everything is in order
			OverlayCanvasRenderTarget->ResizeTarget(CanvasSize.X, CanvasSize.Y);
			OverlayCanvasRenderTarget->UpdateResource();
			bShowOverlay = true;
		}
	}

	GroundOverlay->SetVisibility(bShowOverlay);
}

void ATopDownPlayerController::DrawGroundOverlay(UCanvas* InCanvas, int32 Width, int32 Height)
{
	InCanvas->Canvas->PushRelativeTransform(OverlayCanvasTransform);

	if (!IsPendingKill() && OnOverlayUpdate.IsBound())
	{
		OnOverlayUpdate.Broadcast(InCanvas, Width, Height);
	}

	ReceiveOverlayUpdate(InCanvas, Width, Height);

	InCanvas->Canvas->PopTransform();
}
