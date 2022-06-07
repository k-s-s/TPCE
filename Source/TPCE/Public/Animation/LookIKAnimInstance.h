// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "TPCETypes.h"

#include "LookIKAnimInstance.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;
class UCurveFloat;

/**
 * AnimInstance class related to looking behavior.
 */
UCLASS(abstract)
class TPCE_API ULookIKAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	ULookIKAnimInstance();

	/** Location in world space to look at. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Transient, Category="Animation|Looking")
	FVector LookAtLocation;

	/**
	 * Name of the socket to treat as source. Normally this will be at eye level and parented to a spine bone.
	 * X axis (red arrow) should point forward and Z axis (blue arrow) up.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Looking")
	FName SourceBoneName;

	/** Headlook weight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0", ClampMax="1", UIMax="1"))
	float UseHeadlook;

	/** Bodylook weight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0", ClampMax="1", UIMax="1"))
	float UseBodylook;

	/** Distance between the eyes individual targets when looking at a medium distance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings|Looking")
	float EyeDivergence;

	/** Maximum spine twist angle in degrees. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float BodyTwistMax;

	/** Maximum distance to the look target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float MaxDistance;

	/** How fast to reach the desired look distance. Use 0 for immediate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float DistanceInterpSpeed;

	/** Upper bound is the yaw difference at which linear interpolation kicks in, maximum pitch drop, etc. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking")
	FBounds SwivelRange;

	/** OnSwivel is fired once when SwivelScale passes this threshold, and won't be fired again until SwivelScale returns to zero. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0", ClampMax="1", UIMax="1"))
	float SwivelEventThreshold;

	/** Angle in degrees of the blind spot behind the character.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float YawDeadzone;

	/** Stiffness when reaching the desired look yaw. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float YawStiffness;

	/** Damping when reaching the desired look yaw. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float YawDamping;

	/** How fast to reach the desired look yaw when not using spring. Use 0 for immediate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float YawInterpSpeed;

	/** Pitch drop at maximum yaw difference. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking")
	float PitchDrop;

	/** Stiffness when reaching the desired look pitch. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float PitchStiffness;

	/** Damping when reaching the desired look pitch. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float PitchDamping;

	/** How fast to reach the desired look pitch when not using spring. Use 0 for immediate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float PitchInterpSpeed;

	/** Constant head pitch offset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking")
	float HeadPitchOffset;

	/** Downwards head pitch offset when looking up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking")
	float HeadDownLookingUp;

	/** Upwards head pitch offset when looking down. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking")
	float HeadUpLookingDown;

	/** How fast to reach the desired head yaw. Use 0 for immediate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float HeadYawInterpSpeed;

	/** How much to turn the head up and down when looking vertically. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking")
	float HeadPitchMultiplier;

	/** Maximum allowed delta time for springs. If 0 clamping is disabled. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float MaxDeltaTime;

	/** Scales all interpolation speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Looking", meta=(ClampMin="0", UIMin="0"))
	float GlobalSpeed;

	/**
	 * Called per tick to allow Blueprint to modify the look at location.
	 * @param	DeltaTime				Change in time since last update
	 * @param	InLookAtLocation		The look at location that was passed in.
	 * @param	NewLookAtLocation		(out) The modified look at location.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Looking")
	void ModifyLookAtLocation(float DeltaTime, FVector InLookAtLocation, FVector& NewLookAtLocation);

private:

	/** */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category="References", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* OwnerMesh;

	UPROPERTY(Transient)
	FFloatSpringState LookYawSpringState;

	UPROPERTY(Transient)
	FFloatSpringState LookPitchSpringState;

protected:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** Current look target in world space. Individual eye targets should be made relative to this transform to receive alignment and divergence. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	FTransform LookAtTarget;

	/** Current angular offset from character rotation to look rotation. X is Yaw, Y is Pitch, Z is Distance. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	FVector LookAimOffset;

	/** Angular offset from character rotation to head rotation. X is Yaw, Y is Pitch. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	FVector2D HeadAimOffset;

	/** Current spine twist. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	FRotator SpineTwist;

	/** Value representing the difference between the current and desired looking yaw. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	float SwivelScale;

	/** Indicates that a swivel event occurred. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Animation|Looking")
	uint32 bSwivelFired : 1;

	/** Called when look swivels, may be used to trigger a blink animation. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnSwivel();

public:

	FORCEINLINE USkeletalMeshComponent* GetOwnerMesh() const { return OwnerMesh; }
};
