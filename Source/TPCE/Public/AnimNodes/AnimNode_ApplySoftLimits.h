// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BoneContainer.h"
#include "BonePose.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"

#include "AnimNode_ApplySoftLimits.generated.h"

class USkeletalMeshComponent;

USTRUCT(BlueprintInternalUseOnly)
struct TPCE_API FAnimNode_ApplySoftLimits : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY();

	FAnimNode_ApplySoftLimits();

	/** Name of bone to control. */
	UPROPERTY(EditAnywhere, Category=Angular)
	FBoneReference BoneToModify;

	/** Space to convert transforms into prior to clamping. */
	UPROPERTY(EditAnywhere, Category=Angular)
	TEnumAsByte<EBoneControlSpace> ControlSpace;

	/** Minimum Yaw, Pitch and Roll. */
	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"), meta=(PinHiddenByDefault))
	FVector LimitMin;

	/** Maximum Yaw, Pitch and Roll. */
	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"), meta=(PinHiddenByDefault))
	FVector LimitMax;

	/** Maximum Yaw, Pitch and Roll. */
	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="0", UIMax="180", ClampMin="0", ClampMax="180"), meta=(PinHiddenByDefault))
	FVector Knee;

	/** Flip X limits. */
	UPROPERTY(EditAnywhere, Category=Angular, meta=(DisplayName="X"))
	bool bFlipX;

	/** Flip Y limits. */
	UPROPERTY(EditAnywhere, Category=Angular, meta=(DisplayName="Y"))
	bool bFlipY;

	/** Flip Z limits. */
	UPROPERTY(EditAnywhere, Category=Angular, meta=(DisplayName="Z"))
	bool bFlipZ;

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

	/**
	 * Draws an arc using lines.
	 *
	 * @param	Base			Center of the circle.
	 * @param	X				Normalized axis from one point to the center.
	 * @param	Y				Normalized axis from other point to the center.
	 * @param   MinAngle        The minimum angle.
	 * @param   MaxAngle        The maximum angle.
	 * @param   Radius          Radius of the arc.
	 * @param	Sections		Numbers of sides that the circle has.
	 * @param	Color			Color of the circle.
	 * @param	DepthPriority	Depth priority for the circle.
	 */
	void DrawDebugArc(FPrimitiveDrawInterface* PDI, const FVector Base, const FVector& X, const FVector& Y, const float MinAngle, const float MaxAngle,
		const float Radius, const int32 Sections, const FColor& Color, uint8 DepthPriority = 0, float Thickness = 0.f) const;
#endif // WITH_EDITOR

private:
	// Begin FAnimNode_SkeletalControlBase Interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End FAnimNode_SkeletalControlBase Interface

#if !UE_BUILD_SHIPPING
	/** Debug draw cached data. */
	FTransform CachedBoneTM;
#endif
};
