// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Components/PushToTargetComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Logging/LogMacros.h"
#include "Logging/MessageLog.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "DrawDebugHelpers.h"
#include "Math/MathExtensions.h"

#define LOCTEXT_NAMESPACE "PushToTarget"
DEFINE_LOG_CATEGORY_STATIC(LogPushToTarget, Log, All);

const float UPushToTargetComponent::MIN_TICK_TIME = 1e-6f;

UPushToTargetComponent::UPushToTargetComponent()
{
	Speed = 0.0f;
	RotationSpeed = 0.0f;
	RotationType = EPushToTargetRotationType::None;
	bTeleportToTargetToStart = false;
	bForceSubStepping = true;
	MaxSimulationTimeStep = 1.f / 30.f;
}

void UPushToTargetComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Validate properties
	Speed = FMath::Clamp(Speed, 0.f, 10000.f);
	MaxSimulationTimeStep = FMath::Clamp(MaxSimulationTimeStep, 1.f / 60.f, 0.500f);
}

bool UPushToTargetComponent::MoveUpdatedComponent(const FVector& Delta, const FQuat& NewRotation)
{
	FHitResult Hit(1.f);

	// Move the updated component
	bIsBlocked = !SafeMoveUpdatedComponent(Delta, NewRotation, true, Hit, ETeleportType::TeleportPhysics);

	// If we hit a trigger that destroyed the updated component, clean up and abort
	if (!IsValid(UpdatedComponent))
	{
		SetUpdatedComponent(nullptr);
		return false;
	}

	// If we hit a trigger that destroyed the target component, abort
	if (!bTargetLocationSet && !TargetComponent.IsValid())
	{
		return false;
	}

	// Handle hit result after movement
	if (bSlide && Hit.bBlockingHit)
	{
		if (SlideAlongSurface(Delta * (1.0f - FMath::Clamp(Friction, 0.0f, 1.0f)), 1.0f - Hit.Time, Hit.ImpactNormal, Hit, true) < KINDA_SMALL_NUMBER)
			return false;
	}

	// If we hit a trigger that destroyed the updated component, clean up and abort
	if (!IsValid(UpdatedComponent))
	{
		SetUpdatedComponent(nullptr);
		return false;
	}

	// If we hit a trigger that destroyed the target component, abort
	if (!bTargetLocationSet && !TargetComponent.IsValid())
	{
		return false;
	}

	return true;
}

void UPushToTargetComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_PushToTargetMovementComponent_TickComponent);

	if (ShouldSkipUpdate(DeltaTime))
		return;

	// Not a typo. Skip the UMovementComponent::TickComponent cause we're going to repeat the test right next.
	Super::Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UpdatedComponent->IsPendingKill())
	{
		SetUpdatedComponent(nullptr);
		Velocity = FVector::ZeroVector;
		UpdateComponentVelocity();
		return;
	}

	if (UpdatedComponent->IsSimulatingPhysics() || (!bTargetLocationSet && !TargetComponent.IsValid()) || !IsStillInWorld())
	{
		Velocity = FVector::ZeroVector;
		UpdateComponentVelocity();
		return;
	}

	const float InverseTargetLagMaxTimeStep = 1.f / MaxSimulationTimeStep;

	FVector FinalDesiredLocation = GetTargetLocation();
	FVector DesiredLocation = FinalDesiredLocation;
	FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	FVector AdjustedLocation = AdjustCurrentLocationToTarget(CurrentLocation, DesiredLocation);

	FRotator FinalDesiredRotation = GetTargetRotation(CurrentLocation);
	FRotator DesiredRotation = FinalDesiredRotation;
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();
	FRotator AdjustedRotation = AdjustCurrentRotationToTarget(CurrentRotation, DesiredRotation);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugMarkers)
	{
		DrawDebugSphere(GetWorld(), FinalDesiredLocation, 5.0f, 8, FColor::Red);
	}
#endif

	if (bEnableLag)
	{
		if (bForceSubStepping && DeltaTime > MaxSimulationTimeStep)
		{
			const FVector TargetMovementStep = (FinalDesiredLocation - PreviousDesiredLocation) * (1.0f / DeltaTime);
			const FRotator TargetRotationStep = (FinalDesiredRotation - PreviousDesiredRotation) * (1.0f / DeltaTime);

			FVector LerpLocationTarget = PreviousDesiredLocation;
			FRotator LerpRotationTarget = PreviousDesiredRotation;
			float RemainingTime = DeltaTime;
			bool bKeepMoving = true;
			while (bKeepMoving && RemainingTime > MIN_TICK_TIME)
			{
				// Calculate desired location
				const float LerpAmount = FMath::Min(MaxSimulationTimeStep, RemainingTime);
				LerpLocationTarget += TargetMovementStep * (LerpAmount * InverseTargetLagMaxTimeStep);
				LerpRotationTarget += TargetRotationStep * (LerpAmount * InverseTargetLagMaxTimeStep);
				RemainingTime -= LerpAmount;
				DesiredLocation = VInterpTo(AdjustedLocation, LerpLocationTarget, LerpAmount, Speed);
				DesiredRotation = RInterpTo(AdjustedRotation, LerpRotationTarget, LerpAmount, RotationSpeed);
				// Perform Move
				bKeepMoving = MoveUpdatedComponent(ConstrainDirectionToPlane(DesiredLocation - CurrentLocation), CurrentRotation.Quaternion());
				// Update Velocity (for the record) and locations for next iteration
				if (UpdatedComponent)
				{
					const FVector NewLocation = UpdatedComponent->GetComponentLocation();
					const FRotator NewRotation = UpdatedComponent->GetComponentRotation();
					// Update Velocity
					Velocity = (NewLocation - CurrentLocation) / LerpAmount;
					// Get the actual updated component location every time as it could have been constrained to a plane or blocked
					AdjustedLocation = CurrentLocation = NewLocation;
					AdjustedRotation = CurrentRotation = NewRotation;
				}
				else
				{
					Velocity = FVector::ZeroVector;
				}
			}

			// Avoid to unecessarily move the updated component again since it's already been moved in the substeps.
			goto UpdatedComponentMoved;
		}
		else
		{
			DesiredLocation = VInterpTo(AdjustedLocation, DesiredLocation, DeltaTime, Speed);
			DesiredRotation = RInterpTo(AdjustedRotation, DesiredRotation, DeltaTime, RotationSpeed);
		}
	}

	// NOTE: skipped if lag sub-stepping was executed above
	{
		MoveUpdatedComponent(ConstrainDirectionToPlane(DesiredLocation - CurrentLocation), DesiredRotation.Quaternion());
		Velocity = UpdatedComponent ? (UpdatedComponent->GetComponentLocation() - CurrentLocation) / DeltaTime : FVector::ZeroVector;
	}

UpdatedComponentMoved:
	PreviousDesiredLocation = FinalDesiredLocation;
	PreviousDesiredRotation = FinalDesiredRotation;

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugMarkers)
	{
		if (UpdatedComponent)
			DrawDebugSphere(GetWorld(), UpdatedComponent->GetComponentLocation(), 5.0f, 8, FColor::Orange);
	}
#endif

	UpdateComponentVelocity();
}

bool UPushToTargetComponent::IsStillInWorld()
{
	checkf(IsValid(UpdatedComponent), TEXT("UpdatedComponent is assumed to be valid when calling CheckStillInWorld to avoid redundant checks."));

	const UWorld* MyWorld = GetWorld();
	if (!MyWorld)
	{
		return false;
	}

	// check the variations of KillZ
	AWorldSettings* WorldSettings = MyWorld->GetWorldSettings(true);
	if (!WorldSettings->bEnableWorldBoundsChecks)
	{
		return true;
	}

	AActor* ActorOwner = UpdatedComponent->GetOwner();
	if (!IsValid(ActorOwner))
	{
		return false;
	}

	if (ActorOwner->GetActorLocation().Z < WorldSettings->KillZ)
	{
		UDamageType const* DmgType = WorldSettings->KillZDamageType ? WorldSettings->KillZDamageType->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
		ActorOwner->FellOutOfWorld(*DmgType);
		return false;
	}
	// Check if box has poked outside the world
	else if (UpdatedComponent && UpdatedComponent->IsRegistered())
	{
		const FBox&	Box = UpdatedComponent->Bounds.GetBox();
		if (Box.Min.X < -HALF_WORLD_MAX || Box.Max.X > HALF_WORLD_MAX ||
			Box.Min.Y < -HALF_WORLD_MAX || Box.Max.Y > HALF_WORLD_MAX ||
			Box.Min.Z < -HALF_WORLD_MAX || Box.Max.Z > HALF_WORLD_MAX)
		{
			UE_LOG(LogPushToTarget, Warning, TEXT("%s is outside the world bounds!"), *ActorOwner->GetName());
			ActorOwner->OutsideWorldBounds();
			// not safe to use physics or collision at this point
			ActorOwner->SetActorEnableCollision(false);
			return false;
		}
	}

	return true;
}

void UPushToTargetComponent::SetTargetComponent(USceneComponent* NewTargetComponent, const FName SocketName)
{
	if (TargetComponent != NewTargetComponent || TargetSocketName != SocketName)
	{
		TargetComponent = NewTargetComponent;
		TargetSocketName = SocketName;
		TargetLocation = FVector::ZeroVector;
		bTargetLocationSet = false;

		if (bTeleportToTargetToStart)
		{
			SnapToTarget();
		}
	}
}

void UPushToTargetComponent::SetTargetLocation(FVector NewTargetLocation)
{
	TargetComponent.Reset();
	TargetSocketName = NAME_None;
	TargetLocation = NewTargetLocation;
	bTargetLocationSet = true;

	if (bTeleportToTargetToStart)
	{
		SnapToTarget();
	}
}

void UPushToTargetComponent::ClearTarget()
{
	TargetComponent.Reset();
	TargetSocketName = NAME_None;
	TargetLocation = FVector::ZeroVector;
	bTargetLocationSet = false;
}

void UPushToTargetComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	const bool bIsNewValue = UpdatedComponent != NewUpdatedComponent;

	Super::SetUpdatedComponent(NewUpdatedComponent);

	if (bIsNewValue)
	{
		bIsBlocked = false;
		if (bTeleportToTargetToStart)
		{
			SnapToTarget();
		}
	}
}

void UPushToTargetComponent::SnapToTarget()
{
	if (bTargetLocationSet || TargetComponent.IsValid())
	{
		const FVector NewTargetLocation = GetTargetLocation();
		const FRotator NewTargetRotation = GetTargetRotation(NewTargetLocation);
		PreviousDesiredLocation = NewTargetLocation;
		PreviousDesiredRotation = NewTargetRotation;
		if (IsValid(UpdatedComponent))
		{
			UpdatedComponent->SetWorldLocationAndRotation(ConstrainLocationToPlane(NewTargetLocation), NewTargetRotation.Quaternion(), false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

FVector UPushToTargetComponent::GetTargetLocation() const
{
	if (bTargetLocationSet)
	{
		return TargetLocation;
	}
	else if (TargetComponent.IsValid())
	{
		const FTransform SocketTransform = TargetComponent->GetSocketTransform(TargetSocketName);
		const FTransform ActorTransform = TargetComponent->GetOwner()->GetActorTransform();

		FVector NewTargetLocation = SocketTransform.GetLocation()
			+ SocketTransform.TransformVector(SocketRelativeOffset)
			+ ActorTransform.TransformVector(ActorRelativeOffset)
			+ WorldOffset;

		if (APawn* TargetPawn = Cast<APawn>(TargetComponent->GetOwner()))
		{
			NewTargetLocation += TargetPawn->GetControlRotation().RotateVector(ControllerRelativeOffset);
		}

		return NewTargetLocation;
	}

	return FVector::ZeroVector;
}

FRotator UPushToTargetComponent::GetTargetRotation(const FVector& InCurrentLocation) const
{
	if (!IsValid(UpdatedComponent))
	{
		return FRotator::ZeroRotator;
	}

	FRotator NewTargetRotation = UpdatedComponent->GetComponentRotation();

	if (RotationType == EPushToTargetRotationType::FaceTarget)
	{
		FVector TargetLocationNoOffset;
		if (bTargetLocationSet)
		{
			TargetLocationNoOffset = TargetLocation;
		}
		else if (TargetComponent.IsValid())
		{
			TargetLocationNoOffset = TargetComponent->GetSocketLocation(TargetSocketName);
		}

		const FVector DeltaLocation = TargetLocationNoOffset - InCurrentLocation;
		if (!DeltaLocation.IsNearlyZero())
		{
			NewTargetRotation = FRotationMatrix::MakeFromX(DeltaLocation).Rotator();
		}
	}
	else if (RotationType == EPushToTargetRotationType::FaceView)
	{
		if (UWorld* LocalWorld = GetWorld())
		{
#if WITH_EDITOR
			// Getting the viewport camera is trickier than it seems. This is a hack found at https://answers.unrealengine.com/questions/124563/
			if (LocalWorld->ViewLocationsRenderedLastFrame.Num() > 0)
			{
				const FVector TargetViewLocation = LocalWorld->ViewLocationsRenderedLastFrame[0];
				const FVector DeltaLocation = TargetViewLocation - InCurrentLocation;
				NewTargetRotation = FRotationMatrix::MakeFromX(DeltaLocation).Rotator();
			}
			else
#endif
			if (ULocalPlayer* LocalPlayer = LocalWorld->GetFirstLocalPlayerFromController())
			{
				const FVector TargetViewLocation = LocalPlayer->LastViewLocation;
				const FVector DeltaLocation = TargetViewLocation - InCurrentLocation;
				NewTargetRotation = FRotationMatrix::MakeFromX(DeltaLocation).Rotator();
			}
		}
	}

	if (bStayUpright)
	{
		NewTargetRotation.Roll = 0.0f;
		NewTargetRotation.Pitch = 0.0f;
	}

	return NewTargetRotation;
}

void UPushToTargetComponent::GetDesiredLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation = GetTargetLocation();
	OutRotation = GetTargetRotation(UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector);
}

FVector UPushToTargetComponent::AdjustCurrentLocationToTarget(const FVector& InCurrentLocation, const FVector& InTargetLocation) const
{
	return InCurrentLocation;
}

FRotator UPushToTargetComponent::AdjustCurrentRotationToTarget(const FRotator& InCurrentRotation, const FRotator& InTargetRotation) const
{
	return InCurrentRotation;
}

FVector UPushToTargetComponent::VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed)
{
	return FMathEx::VSafeInterpTo(Current, Target, DeltaTime, InterpSpeed);
}

FRotator UPushToTargetComponent::RInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed)
{
	return FMathEx::RSafeInterpTo(Current, Target, DeltaTime, InterpSpeed);
}

#undef LOCTEXT_NAMESPACE