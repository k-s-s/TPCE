// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BoneContainer.h"
#include "BonePose.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "PhysicsEngine/SphylElem.h"

#include "AnimNode_ArmSeparation.generated.h"

class USkeletalMeshComponent;

/**
 * Pushes a bone away from a collider shape by pivoting another joint.
 * Useful to prevent hands from intersecting thighs caused by retargeting between different body types.
 */
USTRUCT(BlueprintInternalUseOnly)
struct TPCE_API FAnimNode_ArmSeparation : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY();

	FAnimNode_ArmSeparation();

	/** Name of bone to control. */
	UPROPERTY(EditAnywhere, Category=ArmSeparation)
	FBoneReference PivotBone;

	/** Name of bone that will intersect the collider. */
	UPROPERTY(EditAnywhere, Category=ArmSeparation)
	FBoneReference EndEffectorBone;

	/** Radius of the bone sphere. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ArmSeparation, meta=(PinHiddenByDefault))
	float EndEffectorRadius;

	/** Exponent applied to the penetration factor. Use values > 1.0 to soften the displacement when lightly touching. */
	UPROPERTY(EditAnywhere, Category=ArmSeparation, meta=(ClampMin="0", UIMin="0"))
	float SmoothDisplacement;

	/** Name of bone to attach the collider. */
	UPROPERTY(EditAnywhere, Category=CapsuleCollider)
	FBoneReference ColliderBone;

	/** Position of the capsule's origin in the bone's local space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CapsuleCollider)
	FVector CapsuleOffset;

	/** Rotation of the capsule. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CapsuleCollider, meta=(PinHiddenByDefault))
	FRotator CapsuleRotation;

	/** Radius of the capsule. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CapsuleCollider, meta=(PinHiddenByDefault))
	float CapsuleRadius;

	/** Add Radius to both ends to find total length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CapsuleCollider, meta=(PinHiddenByDefault))
	float CapsuleLength;

	// Begin FAnimNode_Base Interface
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	virtual bool NeedsOnInitializeAnimInstance() const override { return true; }
	// End FAnimNode_Base Interface

	// Begin FAnimNode_SkeletalControlBase Interface
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// End FAnimNode_SkeletalControlBase Interface

#if WITH_EDITOR
	void ConditionalDebugDraw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* MeshComp) const;
#endif // WITH_EDITOR

private:
	// Begin FAnimNode_SkeletalControlBase Interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End FAnimNode_SkeletalControlBase Interface

	/** Similar to FKSphylElem::GetClosestPointAndNormal but calculates penetration instead. */
	float GetClosestPointAndNormalFromInside(const FVector& Position, const FTransform& BoneTM, FVector& ClosestPosition, FVector& Normal) const;

	/** Utility function that builds a FKSphylElem from the current data. */
	FKSphylElem GetColliderSphylElem() const;

#if !UE_BUILD_SHIPPING
	/** Debug draw cached data. */
	FTransform CachedEndEffectorBoneTM;
	FTransform CachedColliderBoneTM;
	float CachedDisplacementAlpha;

	friend class UAnimGraphNode_ArmSeparation;
#endif
};
