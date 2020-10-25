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
	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"))
	FVector HardMin;

	/** Minimum Yaw, Pitch and Roll where decrement begins to taper off. */
	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"))
	FVector SoftMin;

	/** Maximum Yaw, Pitch and Roll where increment begins to taper off. */
	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"))
	FVector SoftMax;

	/** Maximum Yaw, Pitch and Roll. */
	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"))
	FVector HardMax;

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

private:
	// Begin FAnimNode_SkeletalControlBase Interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End FAnimNode_SkeletalControlBase Interface
};
