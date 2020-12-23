// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Components/ReboundSpringArmComponent.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Engine/World.h"
#include "Math/MathExtensions.h"

UReboundSpringArmComponent::UReboundSpringArmComponent()
{
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bStartAtFullLength = true;
	ProbeSize = 12.0f;
	ProbeChannel = ECC_Camera;
	bDoCollisionTest = true;
	bEnableReboundLag = false;
	ReboundLagSpeed = 5.0f;
}

void UReboundSpringArmComponent::OnRegister()
{
	Super::OnRegister();

	// Enforce reasonable limits to avoid potential div-by-zero
	ReboundLagSpeed = FMath::Max(ReboundLagSpeed, 0.f);
}

void UReboundSpringArmComponent::InitializeComponent()
{
	Super::InitializeComponent();

	PreviousTargetArmLength = bStartAtFullLength ? TargetArmLength : 0.0f;
}

FVector UReboundSpringArmComponent::CalcTargetArm(const FVector& Origin, const FRotator& Rotation, float DeltaTime)
{
	// Calculate socket offset in local space.
	const FVector LocalOffset = FRotationMatrix(Rotation).TransformVector(SocketOffset);
	const FVector DesiredArmVector = Rotation.Vector() * TargetArmLength - LocalOffset;
	float MaxTargetArmLength = DesiredArmVector.Size();

	if (!FMath::IsNearlyZero(MaxTargetArmLength))
	{
		if (bDoCollisionTest)
		{
			bIsCameraFixed = true;

			// Calculate desired socket location.
			const FVector DesiredLoc = Origin - DesiredArmVector;
			// Do a sweep to ensure we are not penetrating the world
			const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
			FHitResult Hit;
			if (GetWorld()->SweepSingleByChannel(Hit, Origin, DesiredLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams))
			{
				const FVector FixedArmVector = Origin - Hit.Location;
				MaxTargetArmLength = FMath::Sign(TargetArmLength) * FixedArmVector.Size();
				if (FMath::Abs(MaxTargetArmLength) < FMath::Abs(PreviousTargetArmLength))
				{
					PreviousTargetArmLength = MaxTargetArmLength;
				}
			}
		}

		PreviousTargetArmLength = bEnableReboundLag ? FMathEx::FSafeInterpTo(PreviousTargetArmLength, MaxTargetArmLength, DeltaTime, ReboundLagSpeed) : MaxTargetArmLength;
		return Rotation.Vector() * PreviousTargetArmLength - LocalOffset * (PreviousTargetArmLength / DesiredArmVector.Size());
	}

	return -LocalOffset;
}

FVector UReboundSpringArmComponent::GetUnfixedCameraPosition() const
{
	return UnfixedCameraPosition;
}

bool UReboundSpringArmComponent::IsCollisionFixApplied() const
{
	return bIsCameraFixed;
}
