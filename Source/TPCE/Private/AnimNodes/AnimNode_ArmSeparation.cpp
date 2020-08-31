// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNodes/AnimNode_ArmSeparation.h"

#include "AnimationCoreLibrary.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"

FAnimNode_ArmSeparation::FAnimNode_ArmSeparation()
	: EndEffectorRadius(10.0f)
	, SmoothDisplacement(5.0f)
	, CapsuleOffset(-20.0f, 0.0f, 0.0f)
	, CapsuleRotation(0.0f, 90.0f, 0.0f)
	, CapsuleRadius(30.0f)
	, CapsuleLength(30.0f)
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

float FAnimNode_ArmSeparation::GetClosestPointAndNormalFromInside(const FVector& Position, const FTransform& BoneTM, FVector& ClosestPosition, FVector& Normal) const
{
	// Similar to FKSphylElem::GetClosestPointAndNormal but calculates penetration instead
	const FKSphylElem ScaledSphyl = GetColliderSphylElem().GetFinalScaled(BoneTM.GetScale3D(), FTransform::Identity);

	const FTransform LocalTM = ScaledSphyl.GetTransform() * BoneTM;
	const FVector ErrorScale = LocalTM.GetScale3D();
	const FVector LocalPosition = LocalTM.InverseTransformPositionNoScale(Position);

	const float HalfLength = 0.5f * ScaledSphyl.Length;
	const float TargetZ = FMath::Clamp(LocalPosition.Z, -HalfLength, HalfLength);	//We want to move to a sphere somewhere along the capsule axis

	const FVector Sphere = LocalTM.TransformPositionNoScale(FVector(0.f, 0.f, TargetZ));
	const FVector Dir = Sphere - Position;
	const float DistToCenter = Dir.Size();
	const float DistToEdge = DistToCenter - ScaledSphyl.Radius;

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

	return bIsInside ? -DistToEdge : 0.0f;
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
	FVector ClosestPosition, Normal;
	const float DistToEdge = GetClosestPointAndNormalFromInside(EndEffectorLocation, ColliderBoneTM, ClosestPosition, Normal);
	float DisplacementAlpha = 0.0f;

	if (DistToEdge > 0.0f && CapsuleRadius + EndEffectorRadius > 0.0f)
	{
		DisplacementAlpha = FMath::Pow(DistToEdge / (CapsuleRadius + EndEffectorRadius), SmoothDisplacement);
		const FVector PivotLocation = PivotBoneTM.GetLocation();
		const FQuat DeltaRotation = FQuat::FindBetween(PivotLocation - EndEffectorLocation, PivotLocation - ClosestPosition);
		PivotBoneTM.ConcatenateRotation(FQuat::Slerp(FQuat::Identity, DeltaRotation, DisplacementAlpha));

		OutBoneTransforms.Add(FBoneTransform(PivotBoneIndex, PivotBoneTM));
	}

#if !UE_BUILD_SHIPPING
	CachedDisplacementAlpha = DisplacementAlpha;
	CachedEndEffectorBoneTM = EndEffectorBoneTM;
	CachedColliderBoneTM = ColliderBoneTM;
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
		SphylElem.DrawElemWire(PDI, SphylElem.GetTransform() * ColliderWorldTM, ColliderWorldTM.GetScale3D(), FColor::Yellow);
	}
}
#endif
