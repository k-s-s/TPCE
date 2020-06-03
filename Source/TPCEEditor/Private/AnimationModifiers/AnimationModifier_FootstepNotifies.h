// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "Animation/AnimTypes.h"

#include "AnimationModifier_FootstepNotifies.generated.h"

class UAnimSequence;

/**
 * Animation Modifier to generate notifies on foot contacts.
 */
UCLASS(meta = (DisplayName = "Footstep Notifies"))
class UAnimationModifier_FootstepNotifies : public UAnimationModifier
{
	GENERATED_BODY()

public:
	static const FName NotifyTrackName;

	UAnimationModifier_FootstepNotifies();

protected:
	virtual FVector GetBoneWorldLocationAtTime(UAnimSequence* AnimationSequence, FName TargetBoneName, float Time);
	virtual FVector GetRefBoneWorldLocation(const struct FReferenceSkeleton& RefSkel, FName TargetBoneName);

public:
	/** Only animations with a path containing this filter as a case insensitive substring will be affected. Empty value matches all. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FName PathFilter;

	/** Added to reference foot height when the foot is up. Helps prevent jittery animation from triggering extra footsteps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float FootDownThreshold;

	/** Added to reference foot height when the foot is down. Helps prevent jittery animation from triggering extra footsteps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float FootLiftThreshold;

	/** Maximum error in seconds to correct timing of notifies that are evenly spaced in the timeline. Useful for walk cycles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float NudgeEvenThreshold;

	/** An array containing the names of the foot bones. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	TArray<FName> FootBoneNames;

	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
	virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;
};
