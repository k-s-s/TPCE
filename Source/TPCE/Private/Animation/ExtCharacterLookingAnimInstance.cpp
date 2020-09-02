// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Animation/ExtCharacterLookingAnimInstance.h"

#include "GameFramework/ExtCharacter.h"
#include "Math/MathExtensions.h"
#include "Kismet/Kismet.h"
#include "ExtraMacros.h"

UExtCharacterLookingAnimInstance::UExtCharacterLookingAnimInstance()
{
	EyeDivergence = 5.f;
	BodyTwistSoftMax = 20.f;
	BodyTwistHardMax = 60.f;

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

void UExtCharacterLookingAnimInstance::NativeInitializeAnimation()
{
	CharacterOwner = Cast<AExtCharacter>(TryGetPawnOwner());
	CharacterOwnerMesh = GetSkelMeshComponent();
}

void UExtCharacterLookingAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	DeltaSeconds *= GlobalSpeed;

	if (IsValid(CharacterOwner)
		&& IsValid(CharacterOwnerMesh)
		&& DeltaSeconds > 0.f)
	{
		const float UseHeadlook = CharacterOwner->UseHeadlook;
		const float UseBodylook = CharacterOwner->UseBodylook;

		// Break the look vector down into yaw pitch and distance, then make the components reach their targets individually
		// This gives greater control over animation, as well as preventing weird behavior when turning to look at a point diametrically opposite
		// Everything in component space unless otherwise specified
		float NewYaw = AimOffset.X;
		const float MaxYaw = 180.f - YawDeadzone;
		if (FMath::Abs(NewYaw) > MaxYaw)
		{
			// Clamp look yaw. While clamped, the sign is kept to prevent flip-flopping left and right
			NewYaw = MaxYaw * FMath::Sign(LookOffset.X);
		}

		const float YawChange = FMath::Abs(FMath::FindDeltaAngleDegrees(LookOffset.X, NewYaw));
		SwivelScale = FMath::GetMappedRangeValueClamped((FVector2D)SwivelRange, FVector2D(0.f, 1.f), YawChange);
		const float NewPitch = FMath::Clamp(AimOffset.Y, -75.f, 75.f) + SwivelScale * -PitchDrop;
		const float NewDistance = FMath::Min(AimDistance, MaxDistance);

		// Use a spring to move towards the targets because it gives a nice overshoot
		// To prevent it from going overboard use normal interpolation when the difference is too big
		// When the head is locked also prefer a more linear trajectory
		const float ClampedDeltaSeconds = (MaxDeltaTime > 0.f) ? FMath::Min(DeltaSeconds, MaxDeltaTime) : DeltaSeconds;
		const float UseLinearInterp = FMath::Lerp(1.f, SwivelScale, CharacterOwner->UseHeadlook);
		LookOffset.X = FMath::Lerp(
			FMath::Clamp(UKismetMathLibrary::FloatSpringInterp(LookOffset.X, NewYaw, LookYawSpringState, YawStiffness, YawDamping, ClampedDeltaSeconds), -180.f, 180.f),
			FMath::FInterpTo(LookOffset.X, NewYaw, DeltaSeconds, YawInterpSpeed),
			UseLinearInterp);
		LookOffset.Y = FMath::Lerp(
			FMath::Clamp(UKismetMathLibrary::FloatSpringInterp(LookOffset.Y, NewPitch, LookPitchSpringState, PitchStiffness, PitchDamping, ClampedDeltaSeconds), -85.f, 85.f),
			FMath::FInterpTo(LookOffset.Y, NewPitch, DeltaSeconds, PitchInterpSpeed),
			UseLinearInterp);
		LookOffset.Z = FMathEx::FSafeInterpTo(LookOffset.Z, NewDistance, DeltaSeconds, DistanceInterpSpeed);

		// Reconstruct the look target in world space
		LookTarget = UKismetMathLibrary::CreateVectorFromYawPitch(LookOffset.X, LookOffset.Y, LookOffset.Z);
		LookTarget = CharacterOwner->GetActorRotation().RotateVector(LookTarget) + CharacterOwner->GetPawnViewLocation();

		// Update headlook values
		const float TotalLookWeight = UseHeadlook + UseBodylook;
		const float HeadlookWeight = TotalLookWeight <= 1.f ? UseHeadlook : (UseHeadlook / TotalLookWeight);
		HeadAimOffset.X = FMath::FInterpTo(HeadAimOffset.X, FRotator::NormalizeAxis(LookOffset.X * HeadlookWeight), DeltaSeconds, HeadYawInterpSpeed);
		HeadAimOffset.Y = LookOffset.Y * HeadPitchMultiplier + HeadPitchOffset;
		HeadAimOffset.Y += FMath::Clamp(LookOffset.Y / 45.f, 0.f, 1.f) * -HeadDownLookingUp;
		HeadAimOffset.Y += FMath::Clamp(LookOffset.Y / -45.f, 0.f, 1.f) * HeadUpLookingDown;
		HeadAimOffset = HeadAimOffset.ClampAxes(-90.f, 90.f);

		// Update bodylook values
		const float BodylookWeight = TotalLookWeight <= 1.f ? UseBodylook : (UseBodylook / TotalLookWeight);
		const float TwistAmount = UKismetMathLibraryEx::SoftCap(FMath::Abs(LookOffset.X * BodylookWeight), BodyTwistSoftMax, BodyTwistHardMax);
		SpineTwist = FRotator(0.f, TwistAmount * FMath::Sign(LookOffset.X * BodylookWeight), 0.f);

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
