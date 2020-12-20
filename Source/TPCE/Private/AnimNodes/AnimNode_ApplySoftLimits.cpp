// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimNodes/AnimNode_ApplySoftLimits.h"

#include "AnimationCoreLibrary.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "Math/MathExtensions.h"

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

	const FVector FlipFactor = FVector(bFlipX ? -1.f : 1.f, bFlipY ? -1.f : 1.f, bFlipZ ? -1.f : 1.f);
	const FVector Min = (LimitMin * FlipFactor).ComponentMin(LimitMax * FlipFactor);
	const FVector Max = (LimitMin * FlipFactor).ComponentMax(LimitMax * FlipFactor);
	DeltaRot.Yaw = FMathEx::SoftClipRange(DeltaRot.Yaw, Min.X, Max.X, Knee.X);
	// Swapped pitch and roll here because it gives the desired result, real fix would be to let the user specify axes
	DeltaRot.Roll = FMathEx::SoftClipRange(DeltaRot.Roll, Min.Y, Max.Y, Knee.Y);
	DeltaRot.Pitch = FMathEx::SoftClipRange(DeltaRot.Pitch, Min.Z, Max.Z, Knee.Z);

	// Add the reference pose and back to component space
	BoneTM.SetRotation(BoneRefTM.GetRotation() * DeltaRot.Quaternion());
	FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTM, Output.Pose, BoneTM, BoneIndex, ControlSpace);
	OutBoneTransforms.Add(FBoneTransform(BoneIndex, BoneTM));

#if !UE_BUILD_SHIPPING
	CachedBoneTM = FTransform::Identity;
	FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTM, Output.Pose, CachedBoneTM, BoneIndex, ControlSpace);
#endif
}

#if WITH_EDITOR
void FAnimNode_ApplySoftLimits::DrawDebugArc(FPrimitiveDrawInterface* PDI, const FVector Base, const FVector& X, const FVector& Y, const float MinAngle, const float MaxAngle,
	const float Radius, const int32 Sections, const FColor& Color, uint8 DepthPriority, float Thickness) const
{
	const float AngleStep = (MaxAngle - MinAngle) / ((float)(Sections));
	float CurrentAngle = MinAngle;

	FVector LastVertex = Base + Radius * (FMath::Cos(CurrentAngle * (PI / 180.0f)) * X + FMath::Sin(CurrentAngle * (PI / 180.0f)) * Y);
	CurrentAngle += AngleStep;

	for (int32 i = 0; i < Sections; i++)
	{
		FVector ThisVertex = Base + Radius * (FMath::Cos(CurrentAngle * (PI / 180.0f)) * X + FMath::Sin(CurrentAngle * (PI / 180.0f)) * Y);
		PDI->DrawLine(LastVertex, ThisVertex, Color, DepthPriority, Thickness);
		LastVertex = ThisVertex;
		CurrentAngle += AngleStep;
	}
}

void FAnimNode_ApplySoftLimits::ConditionalDebugDraw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* MeshComp) const
{
	if (PDI && MeshComp)
	{
		const FTransform LocalToWorld = MeshComp->GetComponentTransform();
		const FTransform BoneWorldTM = CachedBoneTM * LocalToWorld;

		const FVector Origin = BoneWorldTM.GetLocation();
		const FVector XAxis = BoneWorldTM.GetUnitAxis(EAxis::X);
		const FVector YAxis = BoneWorldTM.GetUnitAxis(EAxis::Y);
		const FVector ZAxis = BoneWorldTM.GetUnitAxis(EAxis::Z);

		const float ArcRadius = 20.f;
		const float ArcThickness = .5f;
		const FVector FlipFactor = FVector(bFlipX ? -1.f : 1.f, bFlipY ? -1.f : 1.f, bFlipZ ? -1.f : 1.f);
		const FVector Min = (LimitMin * FlipFactor).ComponentMin(LimitMax * FlipFactor);
		const FVector Max = (LimitMin * FlipFactor).ComponentMax(LimitMax * FlipFactor);
		DrawDebugArc(PDI, Origin, -YAxis, XAxis, Min.X, Max.X, ArcRadius, 20, FColor::Blue, SDPG_World, ArcThickness);  // Yaw
		DrawDebugArc(PDI, Origin, -YAxis, ZAxis, Min.Y, Max.Y, ArcRadius, 20, FColor::Green, SDPG_World, ArcThickness);  // Pitch
		//DrawDebugArc(PDI, Origin, ZAxis, YAxis, Min.Z, Max.Z, ArcRadius, 20, FColor::Red, SDPG_World, ArcThickness);  // Roll
	}
}
#endif

bool FAnimNode_ApplySoftLimits::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return BoneToModify.IsValidToEvaluate(RequiredBones);
}

void FAnimNode_ApplySoftLimits::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(InitializeBoneReferences)
	BoneToModify.Initialize(RequiredBones);
}
