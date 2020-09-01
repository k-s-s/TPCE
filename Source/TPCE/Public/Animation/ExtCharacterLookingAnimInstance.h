// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "ExtraTypes.h"

#include "ExtCharacterLookingAnimInstance.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;
class UCurveFloat;
class AExtCharacter;
class UExtCharacterMovementComponent;

/**
 * AnimInstance class related to looking behavior.
 */
UCLASS(abstract)
class TPCE_API UExtCharacterLookingAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UExtCharacterLookingAnimInstance();

	/** Input angular offset from character rotation to look rotation. X is Yaw, Y is Pitch. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Transient, Category="Character", meta=(AllowPrivateAccess="true"))
	FVector2D AimOffset;

	/** Input distance to the look target. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Transient, Category="Character", meta=(AllowPrivateAccess="true"))
	float AimDistance;

	/** Distance between the eyes individual targets when looking at a medium distance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true"))
	float EyeDivergence;

	/** Maximum spine twist angle in degrees. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float BodyTwistSoftMax;

	/** Maximum spine twist angle in degrees. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float BodyTwistHardMax;

	/** Maximum distance to the look target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float MaxDistance;

	/** How fast to reach the desired look distance. Use 0 for immediate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float DistanceInterpSpeed;

	/** Yaw difference at which linear interpolation kicks in, maximum pitch drop, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true"))
	float BigYaw;

	/** Angle in degrees of the blind spot behind the character.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float YawDeadzone;

	/** Stiffness when reaching the desired look yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float YawStiffness;

	/** Damping when reaching the desired look yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float YawDamping;

	/** How fast to reach the desired look yaw. Use 0 for immediate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float YawInterpSpeed;

	/** Pitch drop at maximum yaw difference. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true"))
	float PitchDrop;

	/** Stiffness when reaching the desired look pitch. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float PitchStiffness;

	/** Damping when reaching the desired look pitch. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float PitchDamping;

	/** How fast to reach the desired look pitch. Use 0 for immediate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float PitchInterpSpeed;

	/** Constant head pitch offset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true"))
	float HeadPitchOffset;

	/** Downwards head pitch offset in degrees when looking up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true"))
	float HeadDownLookingUp;

	/** Upwards head pitch offset in degrees when looking down. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true"))
	float HeadUpLookingDown;

	/** How fast to reach the desired head yaw. Use 0 for immediate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true", ClampMin="0", UIMin="0"))
	float HeadYawInterpSpeed;

	/** How much to turn the head up and down when looking vertically. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Looking", meta=(AllowPrivateAccess="true"))
	float HeadPitchMultiplier;

private:

	/** */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category="References", meta=(AllowPrivateAccess="true"))
	AExtCharacter* CharacterOwner;

	/** */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category="References", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* CharacterOwnerMesh;

	UPROPERTY(Transient)
	FFloatSpringState LookYawSpringState;

	UPROPERTY(Transient)
	FFloatSpringState LookPitchSpringState;

protected:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** Angular offset from character rotation to look rotation. X is Yaw, Y is Pitch, Z is Distance. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	FVector LookOffset;

	/** Point in world space that the eyes should be looking at. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	FVector LookTarget;

	/** Angular offset from character rotation to head rotation. X is Yaw, Y is Pitch. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	FVector2D HeadAimOffset;

	/** Spine twist. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	FRotator SpineTwist;

	/** Value representing the difference between the current and desired looking yaw. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	float BigYawChange;

public:

	FORCEINLINE AExtCharacter* GetCharacterOwner() const { return CharacterOwner; }

	FORCEINLINE USkeletalMeshComponent* GetCharacterOwnerMesh() const { return CharacterOwnerMesh; }
};
