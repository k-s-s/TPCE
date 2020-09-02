// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/ExtCharacterLookingAnimInstance.h"

#include "GameFramework/ExtCharacter.h"
#include "Math/MathExtensions.h"
#include "Kismet/Kismet.h"
#include "ExtraMacros.h"

UExtCharacterLookingAnimInstance::UExtCharacterLookingAnimInstance()
{
	EyeDivergence = 5.0f;
	BodyTwistSoftMax = 20.0f;
	BodyTwistHardMax = 60.0f;

	MaxDistance = 200.0f;
	DistanceInterpSpeed = 50.0f;

	BigYaw = FBounds(30.0f, 120.0f);
	YawDeadzone = 20.0f;
	YawStiffness = 42.0f;
	YawDamping = 0.4f;
	YawInterpSpeed = 8.0f;

	PitchDrop = 20.0f;
	PitchStiffness = 15.0f;
	PitchDamping = 0.8f;
	PitchInterpSpeed = 6.0f;

	HeadPitchOffset = 0.0f;
	HeadDownLookingUp = 10.0f;
	HeadUpLookingDown = 0.0f;
	HeadYawInterpSpeed = 5.0f;
	HeadPitchMultiplier = 2.0f;
}

void UExtCharacterLookingAnimInstance::NativeInitializeAnimation()
{
	CharacterOwner = Cast<AExtCharacter>(TryGetPawnOwner());
	CharacterOwnerMesh = GetSkelMeshComponent();
}

void UExtCharacterLookingAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (IsValid(CharacterOwner)
		&& IsValid(CharacterOwnerMesh)
		&& DeltaSeconds > 0.0f)
	{
		const float UseHeadlook = CharacterOwner->UseHeadlook;
		const float UseBodylook = CharacterOwner->UseBodylook;

		// Break the look vector down into yaw pitch and distance, then make the components reach their targets individually
		// This gives greater control over animation, as well as preventing weird behavior when turning to look at a point diametrically opposite
		// Everything in component space unless otherwise specified
		float NewYaw = AimOffset.X;
		const float MaxYaw = 180.0f - YawDeadzone;
		if (FMath::Abs(NewYaw) > MaxYaw)
		{
			// Clamp look yaw. While clamped, the sign is kept to prevent flip-flopping left and right
			NewYaw = MaxYaw * FMath::Sign(LookOffset.X);
		}

		const float YawChange = FMath::Abs(FMath::FindDeltaAngleDegrees(LookOffset.X, NewYaw));
		BigYawChange = FMath::GetMappedRangeValueClamped((FVector2D)BigYaw, FVector2D(0.f, 1.f), YawChange);
		const float NewPitch = FMath::Clamp(AimOffset.Y, -75.0f, 75.0f) + BigYawChange * -PitchDrop;
		const float NewDistance = FMath::Min(AimDistance, MaxDistance);

		// Use a spring to move towards the targets because it gives a nice overshoot
		// To prevent it from going overboard use normal interpolation when the difference is too big
		// When the head is locked also prefer a more linear trajectory
		const float UseLinearInterp = FMath::Lerp(1.0f, BigYawChange, CharacterOwner->UseHeadlook);
		LookOffset.X = FMath::Lerp(
			FMath::Clamp(UKismetMathLibrary::FloatSpringInterp(LookOffset.X, NewYaw, LookYawSpringState, YawStiffness, YawDamping, DeltaSeconds), -180.0f, 180.0f),
			FMath::FInterpTo(LookOffset.X, NewYaw, DeltaSeconds, YawInterpSpeed),
			UseLinearInterp);
		LookOffset.Y = FMath::Lerp(
			FMath::Clamp(UKismetMathLibrary::FloatSpringInterp(LookOffset.Y, NewPitch, LookPitchSpringState, PitchStiffness, PitchDamping, DeltaSeconds), -85.0f, 85.0f),
			FMath::FInterpTo(LookOffset.Y, NewPitch, DeltaSeconds, PitchInterpSpeed),
			UseLinearInterp);
		LookOffset.Z = FMathEx::FSafeInterpTo(LookOffset.Z, NewDistance, DeltaSeconds, DistanceInterpSpeed);

		// Reconstruct the look target in world space
		LookTarget = UKismetMathLibrary::CreateVectorFromYawPitch(LookOffset.X, LookOffset.Y, LookOffset.Z);
		LookTarget = CharacterOwner->GetActorRotation().RotateVector(LookTarget) + CharacterOwner->GetPawnViewLocation();

		// Update headlook values
		const float TotalLookWeight = UseHeadlook + UseBodylook;
		const float HeadlookWeight = TotalLookWeight <= 1.0f ? UseHeadlook : (UseHeadlook / TotalLookWeight);
		HeadAimOffset.X = FMath::FInterpTo(HeadAimOffset.X, FRotator::NormalizeAxis(LookOffset.X * HeadlookWeight), DeltaSeconds, HeadYawInterpSpeed);
		HeadAimOffset.Y = LookOffset.Y * HeadPitchMultiplier + HeadPitchOffset;
		HeadAimOffset.Y += FMath::Clamp(LookOffset.Y / 45.0f, 0.0f, 1.0f) * -HeadDownLookingUp;
		HeadAimOffset.Y += FMath::Clamp(LookOffset.Y / -45.0f, 0.0f, 1.0f) * HeadUpLookingDown;
		HeadAimOffset = HeadAimOffset.ClampAxes(-90.0f, 90.0f);

		// Update bodylook values
		const float BodylookWeight = TotalLookWeight <= 1.0f ? UseBodylook : (UseBodylook / TotalLookWeight);
		const float TwistAmount = UKismetMathLibraryEx::SoftCap(FMath::Abs(LookOffset.X * BodylookWeight), BodyTwistSoftMax, BodyTwistHardMax);
		SpineTwist = FRotator(0.0f, TwistAmount * FMath::Sign(LookOffset.X * BodylookWeight), 0.0f);
	}
}
