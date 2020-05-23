// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "Animation/AnimTypes.h"

#include "AnimationModifier_BoneDistance.generated.h"

class UAnimSequence;

USTRUCT(BlueprintType)
struct FBonePair
{
	GENERATED_BODY();

	FBonePair();
	FBonePair(FName BoneName1, FName BoneName2);

	/** Name of the first bone. */
	UPROPERTY(EditAnywhere)
	FName BoneName1;

	/** Name of the second bone. */
	UPROPERTY(EditAnywhere)
	FName BoneName2;

	/** Constant distance offset. */
	UPROPERTY(EditAnywhere)
	float Offset;
};

/**
 * Animation Modifier to generate curves from the distance between two bones. 
 */
UCLASS(meta = (DisplayName = "Bone Distance"))
class UAnimationModifier_BoneDistance : public UAnimationModifier
{
	GENERATED_BODY()

public:
	UAnimationModifier_BoneDistance();

protected:
	virtual FVector GetWorldBoneLocationAtTime(UAnimSequence* AnimationSequence, FName TargetBoneName, float Time);

public:
	/** Only animations with a path containing this filter as a case insensitive substring will be affected. Empty value matches all. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
	FName PathFilter;
	
	/** An array specifying the distances that should be measured. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
	TArray<FBonePair> BonePairs;

	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
	virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;
};
