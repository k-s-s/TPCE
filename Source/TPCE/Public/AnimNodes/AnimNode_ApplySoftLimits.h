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

	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"))
	FVector HardMin;

	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"))
	FVector SoftMin;

	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"))
	FVector SoftMax;

	UPROPERTY(EditAnywhere, Category=Angular, meta=(UIMin="-180", UIMax="180", ClampMin="-180", ClampMax="180"))
	FVector HardMax;

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
