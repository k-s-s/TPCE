// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimNodes/AnimNode_ArmSeparation.h"

#include "AnimationCoreLibrary.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"

FAnimNode_ArmSeparation::FAnimNode_ArmSeparation()
	: EndEffectorRadius(10.f)
	, CapsuleOffset(0.f, 0.f, 0.f)
	, CapsuleRotation(0.f, 90.f, 0.f)
	, CapsuleRadius(30.f)
	, CapsuleLength(30.f)
	, BiasTowardsSide(0.5f)
	, SmoothDisplacement(5.0f)
	, bFlipDisplacement(false)
{
}

void FAnimNode_ArmSeparation::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(GatherDebugData)
	FString DebugLine = DebugData.GetNodeName(this);

	DebugLine += "(";
	AddDebugNodeData(DebugLine);
	DebugLine += FString::Printf(TEXT(")"));
	DebugData.AddDebugItem(DebugLine);

	ComponentPose.GatherDebugData(DebugData);
}

float FAnimNode_ArmSeparation::GetClosestPointAndNormalFromInside(const FVector& Position, const FTransform& BoneTM, FVector& ClosestPosition, FVector& SidePosition, FVector& Normal) const
{
	const FKSphylElem ScaledSphyl = GetColliderSphylElem().GetFinalScaled(BoneTM.GetScale3D(), FTransform::Identity);

	const FTransform LocalTM = ScaledSphyl.GetTransform() * BoneTM;
	const FVector ErrorScale = LocalTM.GetScale3D();
	const FVector LocalPosition = LocalTM.InverseTransformPositionNoScale(Position);

	const float HalfLength = 0.5f * ScaledSphyl.Length;
	const float TargetZ = FMath::Clamp(LocalPosition.Z, -HalfLength, HalfLength);	//We want to move to a sphere somewhere along the capsule axis

	const FVector Sphere = LocalTM.TransformPositionNoScale(FVector(0.f, 0.f, TargetZ));
	const FVector Dir = Sphere - Position;
	const float DistToCenter = Dir.Size();
	const float ScaledRadius = ScaledSphyl.Radius + EndEffectorRadius;
	const float DistToEdge = DistToCenter - ScaledRadius;

	bool bIsInside = DistToEdge < -SMALL_NUMBER;
	if (bIsInside)
	{
		Normal = -Dir.GetUnsafeNormal();
	}
	else
	{
		Normal = FVector::ZeroVector;
	}

	ClosestPosition = Position - Normal * DistToEdge;
	SidePosition = LocalTM.TransformPositionNoScale(FVector(ScaledRadius, 0.f, 0.f));

	return (bIsInside && ScaledRadius > 0.0f) ? (-DistToEdge / ScaledRadius) : 0.f;
}

void FAnimNode_ArmSeparation::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(EvaluateSkeletalControl_AnyThread)
	checkSlow(OutBoneTransforms.Num() == 0);

	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	const FCompactPoseBoneIndex PivotBoneIndex = PivotBone.GetCompactPoseIndex(BoneContainer);
	const FCompactPoseBoneIndex EndEffectorBoneIndex = EndEffectorBone.GetCompactPoseIndex(BoneContainer);
	const FCompactPoseBoneIndex ColliderBoneIndex = ColliderBone.GetCompactPoseIndex(BoneContainer);
	FTransform PivotBoneTM = Output.Pose.GetComponentSpaceTransform(PivotBoneIndex);
	const FTransform EndEffectorBoneTM = Output.Pose.GetComponentSpaceTransform(EndEffectorBoneIndex);
	const FTransform ColliderBoneTM = Output.Pose.GetComponentSpaceTransform(ColliderBoneIndex);
	const FVector EndEffectorLocation = EndEffectorBoneTM.GetLocation();

	const FKSphylElem SphylElem = GetColliderSphylElem();
	FVector ClosestPosition, Normal, SidePosition;
	const float PenetrationFactor = GetClosestPointAndNormalFromInside(EndEffectorLocation, ColliderBoneTM, ClosestPosition, SidePosition, Normal);

	const FVector TargetPosition = FMath::Lerp(ClosestPosition, SidePosition, BiasTowardsSide);
	float DisplacementAlpha = 0.f;

	if (PenetrationFactor > 0.f)
	{
		DisplacementAlpha = FMath::Pow(PenetrationFactor, SmoothDisplacement);
		if (bFlipDisplacement)
		{
			DisplacementAlpha *= -1.f;
		}

		const FVector PivotLocation = PivotBoneTM.GetLocation();
		const FQuat DeltaRotation = FQuat::FindBetween(PivotLocation - EndEffectorLocation, PivotLocation - TargetPosition);
		PivotBoneTM.ConcatenateRotation(FQuat::SlerpFullPath(FQuat::Identity, DeltaRotation, DisplacementAlpha));

		OutBoneTransforms.Add(FBoneTransform(PivotBoneIndex, PivotBoneTM));
	}

#if !UE_BUILD_SHIPPING
	CachedDisplacementAlpha = DisplacementAlpha;
	CachedEndEffectorBoneTM = EndEffectorBoneTM;
	CachedColliderBoneTM = ColliderBoneTM;
	CachedTargetPosition = TargetPosition;
#endif
}

bool FAnimNode_ArmSeparation::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return PivotBone.IsValidToEvaluate(RequiredBones)
		&& EndEffectorBone.IsValidToEvaluate(RequiredBones)
		&& ColliderBone.IsValidToEvaluate(RequiredBones);
}

void FAnimNode_ArmSeparation::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(InitializeBoneReferences)
	PivotBone.Initialize(RequiredBones);
	EndEffectorBone.Initialize(RequiredBones);
	ColliderBone.Initialize(RequiredBones);
}

FKSphylElem FAnimNode_ArmSeparation::GetColliderSphylElem() const
{
	FKSphylElem SphylElem(CapsuleRadius, CapsuleLength);
	SphylElem.Center = CapsuleOffset;
	SphylElem.Rotation = CapsuleRotation;

	return SphylElem;
}

#if WITH_EDITOR
void FAnimNode_ArmSeparation::ConditionalDebugDraw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* MeshComp) const
{
	if (PDI && MeshComp)
	{
		const FTransform LocalToWorld = MeshComp->GetComponentTransform();
		const FTransform EndEffectorBoneWorldTM = CachedEndEffectorBoneTM * LocalToWorld;
		const FTransform ColliderWorldTM = CachedColliderBoneTM * LocalToWorld;

		DrawWireSphere(PDI, EndEffectorBoneWorldTM.GetLocation(), FLinearColor::Red, EndEffectorRadius, 16, SDPG_World);
		const FKSphylElem SphylElem = GetColliderSphylElem();
		SphylElem.DrawElemWire(PDI, SphylElem.GetTransform() * ColliderWorldTM, FVector::OneVector, FColor::Yellow);
		PDI->DrawPoint(CachedTargetPosition, FLinearColor::Yellow, 6.0f, SDPG_World);
	}
}
#endif
