// Copyright (c) 2020 greisane <ggreisane@gmail.com>

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Kismet/KismetMathLibrary.h"

#include "AnimNotifyState_MatchTarget.generated.h"

/**
 * Moves the actor to a target destination and rotation over the notify's duration.
 */
UCLASS(editinlinenew, const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Match Target"))
class TPCE_API UAnimNotifyState_MatchTarget : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_MatchTarget();

	// Begin UAnimNotifyState Interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
#if WITH_EDITOR
	virtual bool CanBePlaced(UAnimSequenceBase* Animation) const override;
#endif
	// End UAnimNotifyState Interface

	/** Name of the function in the AnimBP that should return the location we're moving to. */
	UPROPERTY(EditAnywhere)
	FName MatchTargetGetterName;

	/** If true, interpolate to the target position. */
	UPROPERTY(EditAnywhere)
	bool bMatchPosition;

	/** If true, interpolate to the target rotation. */
	UPROPERTY(EditAnywhere)
	bool bMatchRotation;

	/** If true, interpolate to the target scale. */
	UPROPERTY(EditAnywhere)
	bool bMatchScale;

	/**
	 * Whether we sweep to the destination location.
	 * @see AActor::SetActorTransform
	 */
	UPROPERTY(EditAnywhere)
	bool bSweep;

	/**
	 * How we teleport the physics state.
	 * @see AActor::SetActorTransform
	 */
	UPROPERTY(EditAnywhere)
	ETeleportType Teleport;

	/** Specifies the desired ease function to be applied. */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EEasingFunc::Type> EaseFunction;

	/** Blend exponent for ease function. */
	UPROPERTY(EditAnywhere)
	float BlendExponent;

protected:
	FTransform OriginTM;
	FTransform DestinationTM;
	float Duration;
	float Elapsed;
	TWeakObjectPtr<AActor> Actor;

	FTransform BlendTransform(const FTransform& Origin, const FTransform& Destination, float Alpha) const;
	void UpdateActorTransform(float Alpha);
};
