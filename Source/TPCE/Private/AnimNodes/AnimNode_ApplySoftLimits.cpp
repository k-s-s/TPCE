// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimNodes/AnimNode_ApplySoftLimits.h"

#include "AnimationCoreLibrary.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "Kismet/Kismet.h"

FAnimNode_ApplySoftLimits::FAnimNode_ApplySoftLimits()
	: ControlSpace(BCS_ParentBoneSpace)
{
}

void FAnimNode_ApplySoftLimits::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(GatherDebugData)
	FString DebugLine = DebugData.GetNodeName(this);

	DebugLine += "(";
	AddDebugNodeData(DebugLine);
	DebugLine += FString::Printf(TEXT(")"));
	DebugData.AddDebugItem(DebugLine);

	ComponentPose.GatherDebugData(DebugData);
}

void FAnimNode_ApplySoftLimits::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(EvaluateSkeletalControl_AnyThread)
	checkSlow(OutBoneTransforms.Num() == 0);

	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	const FCompactPoseBoneIndex BoneIndex = BoneToModify.GetCompactPoseIndex(BoneContainer);
	const FTransform& ComponentTM = Output.AnimInstanceProxy->GetComponentTransform();
	const FTransform& BoneRefTM = BoneContainer.GetRefPoseTransform(BoneIndex);
	FTransform BoneTM = Output.Pose.GetComponentSpaceTransform(BoneIndex);

	// Convert to desired space and remove the reference pose
	FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTM, Output.Pose, BoneTM, BoneIndex, ControlSpace);
	FRotator DeltaRot = (BoneRefTM.GetRotation().Inverse() * BoneTM.GetRotation()).Rotator();

	DeltaRot.Yaw = UKismetMathLibraryEx::SoftCap(DeltaRot.Yaw, SoftMin.X, HardMin.X);
	DeltaRot.Yaw = UKismetMathLibraryEx::SoftCap(DeltaRot.Yaw, SoftMax.X, HardMax.X);
	DeltaRot.Pitch = UKismetMathLibraryEx::SoftCap(DeltaRot.Pitch, SoftMin.Y, HardMin.Y);
	DeltaRot.Pitch = UKismetMathLibraryEx::SoftCap(DeltaRot.Pitch, SoftMax.Y, HardMax.Y);
	DeltaRot.Roll = UKismetMathLibraryEx::SoftCap(DeltaRot.Roll, SoftMin.Z, HardMin.Z);
	DeltaRot.Roll = UKismetMathLibraryEx::SoftCap(DeltaRot.Roll, SoftMax.Z, HardMax.Z);

	// Add the reference pose and back to component space
	BoneTM.SetRotation(BoneRefTM.GetRotation() * DeltaRot.Quaternion());
	FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTM, Output.Pose, BoneTM, BoneIndex, ControlSpace);
	OutBoneTransforms.Add(FBoneTransform(BoneIndex, BoneTM));
}

bool FAnimNode_ApplySoftLimits::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return BoneToModify.IsValidToEvaluate(RequiredBones);
}

void FAnimNode_ApplySoftLimits::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(InitializeBoneReferences)
	BoneToModify.Initialize(RequiredBones);
}
