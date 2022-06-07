// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Animation/LookIKAnimInstance.h"

#include "Math/MathExtensions.h"
#include "Kismet/Kismet.h"
#include "TPCEMacros.h"
#include "TPCETypes.h"

ULookIKAnimInstance::ULookIKAnimInstance()
{
	SourceBoneName = FName(TEXT("Eyes"));

	EyeDivergence = 5.f;
	BodyTwistMax = 40.f;

	MaxDistance = 200.f;
	DistanceInterpSpeed = 50.f;

	SwivelRange = FBounds(20.f, 60.f);
	SwivelEventThreshold = 0.25f;
	YawDeadzone = 20.f;
	YawStiffness = 50.f;
	YawDamping = 0.8f;
	YawInterpSpeed = 8.f;

	PitchDrop = 20.f;
	PitchStiffness = 40.f;
	PitchDamping = 1.f;
	PitchInterpSpeed = 6.f;

	HeadPitchOffset = 0.f;
	HeadDownLookingUp = 0.f;
	HeadUpLookingDown = 0.f;
	HeadYawInterpSpeed = 5.f;
	HeadPitchMultiplier = 1.f;

	MaxDeltaTime = 0.2f;
	GlobalSpeed = 1.f;
}

void ULookIKAnimInstance::NativeInitializeAnimation()
{
	OwnerMesh = GetSkelMeshComponent();
}

void ULookIKAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	DeltaSeconds *= GlobalSpeed;

	if (DeltaSeconds > 0.f)
	{
		// Allow blueprint to modify the look at location
		FVector NewLookAtLocation = LookAtLocation;
		ModifyLookAtLocation(DeltaSeconds, LookAtLocation, NewLookAtLocation);

		// Look in source bone space
		FTransform SourceWorldTM = OwnerMesh->GetSocketTransform(SourceBoneName, RTS_World);
		SourceWorldTM.RemoveScaling();
		const FVector LookVector = SourceWorldTM.InverseTransformPositionNoScale(NewLookAtLocation);
		const FRotator LookRotation = FRotationMatrix::MakeFromX(LookVector).Rotator();

		// Break the look vector down into yaw pitch and distance, then make the components reach their targets individually
		// This gives greater control over animation, as well as preventing weird behavior when turning to look at a point diametrically opposite
		float NewYaw = LookRotation.Yaw;
		const float MaxYaw = 180.f - YawDeadzone;
		if (FMath::Abs(NewYaw) > MaxYaw)
		{
			// Clamp look yaw. While clamped, the sign is kept to prevent flip-flopping left and right
			NewYaw = MaxYaw * FMath::Sign(LookAimOffset.X);
		}

		const float YawChange = FMath::Abs(FMath::FindDeltaAngleDegrees(LookAimOffset.X, NewYaw));
		SwivelScale = FMath::GetMappedRangeValueClamped((FVector2D)SwivelRange, FVector2D(0.f, 1.f), YawChange);
		const float NewPitch = FMath::Clamp(LookRotation.Pitch, -75.f, 75.f) + SwivelScale * -PitchDrop;
		const float NewDistance = FMath::Min(LookVector.Size(), MaxDistance);

		// Use a spring to move towards the targets because it gives a nice overshoot
		// To prevent it from going overboard use normal interpolation when the difference is too big
		// When the head is locked also prefer a more linear trajectory
		const float ClampedDeltaSeconds = (MaxDeltaTime > 0.f) ? FMath::Min(DeltaSeconds, MaxDeltaTime) : DeltaSeconds;
		const float UseLinearInterp = FMath::Lerp(1.f, SwivelScale, UseHeadlook);
		LookAimOffset.X = FMath::Lerp(
			FMath::Clamp(UKismetMathLibrary::FloatSpringInterp(LookAimOffset.X, NewYaw, LookYawSpringState, YawStiffness, YawDamping, ClampedDeltaSeconds), -180.f, 180.f),
			FMath::FInterpTo(LookAimOffset.X, NewYaw, DeltaSeconds, YawInterpSpeed),
			UseLinearInterp);
		LookAimOffset.Y = FMath::Lerp(
			FMath::Clamp(UKismetMathLibrary::FloatSpringInterp(LookAimOffset.Y, NewPitch, LookPitchSpringState, PitchStiffness, PitchDamping, ClampedDeltaSeconds), -85.f, 85.f),
			FMath::FInterpTo(LookAimOffset.Y, NewPitch, DeltaSeconds, PitchInterpSpeed),
			UseLinearInterp);
		LookAimOffset.Z = FMathEx::FSafeInterpTo(LookAimOffset.Z, NewDistance, DeltaSeconds, DistanceInterpSpeed);

		// Construct the new look at target in world space
		// The new transform faces the source socket and is scaled according to distance
		// So all the AnimGraph needs is individual LookAts for the eyes with an offset in the local space of this transform
		const float LookTargetScale = UKismetMathLibrary::MapRangeClamped(LookAimOffset.Z, 10.f, 200.f, .3f, 1.f);
		LookAtTarget = FTransform(
			FRotator(LookAimOffset.Y, LookAimOffset.X, 0.f),
			UKismetMathLibrary::CreateVectorFromYawPitch(LookAimOffset.X, LookAimOffset.Y, LookAimOffset.Z),
			FVector(LookTargetScale))
			* SourceWorldTM;

		// Update headlook values
		const float TotalLookWeight = UseHeadlook + UseBodylook;
		const float HeadlookWeight = TotalLookWeight <= 1.f ? UseHeadlook : (UseHeadlook / TotalLookWeight);
		HeadAimOffset.X = FMath::FInterpTo(HeadAimOffset.X, FRotator::NormalizeAxis(LookAimOffset.X * HeadlookWeight), DeltaSeconds, HeadYawInterpSpeed);
		HeadAimOffset.Y = LookAimOffset.Y * HeadPitchMultiplier + HeadPitchOffset;
		HeadAimOffset.Y += FMath::Clamp(LookAimOffset.Y / 45.f, 0.f, 1.f) * -HeadDownLookingUp;
		HeadAimOffset.Y += FMath::Clamp(LookAimOffset.Y / -45.f, 0.f, 1.f) * HeadUpLookingDown;
		HeadAimOffset = HeadAimOffset.ClampAxes(-90.f, 90.f);

		// Update bodylook values
		// Currently this only works left to right and hopes that the aim offset will impart some curve to the spine when looking up and down
		const float BodylookWeight = TotalLookWeight <= 1.f ? UseBodylook : (UseBodylook / TotalLookWeight);
		const float TwistAmount = FMathEx::SoftClipRange(LookAimOffset.X * BodylookWeight, -BodyTwistMax, BodyTwistMax, BodyTwistMax * .5f);
		SpineTwist = FRotator(0.f, TwistAmount, 0.f);

		// Raise events
		if (SwivelScale > SwivelEventThreshold && !bSwivelFired)
		{
			bSwivelFired = true;
			OnSwivel();
		}
		else if (SwivelScale <= 0.f && bSwivelFired)
		{
			bSwivelFired = false;
		}
	}
}
