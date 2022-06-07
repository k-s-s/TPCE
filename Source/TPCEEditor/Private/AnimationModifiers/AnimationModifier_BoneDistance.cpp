// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimationModifiers/AnimationModifier_BoneDistance.h"

#include "Animation/AnimSequence.h"
#include "UObject/UObjectBaseUtility.h"
#include "TPCETypes.h"

FBonePair::FBonePair()
	: BoneName1(NAME_None)
	, BoneName2(NAME_None)
	, Offset(0.0f)
{
}

FBonePair::FBonePair(FName InBoneName1, FName InBoneName2)
	: BoneName1(InBoneName1)
	, BoneName2(InBoneName2)
	, Offset(1.0f)
{
}

UAnimationModifier_BoneDistance::UAnimationModifier_BoneDistance()
{
	BonePairs.Add(FBonePair(NAME_Thigh_L, NAME_Hand_L));
	BonePairs.Add(FBonePair(NAME_Thigh_R, NAME_Hand_R));
}

FVector UAnimationModifier_BoneDistance::GetWorldBoneLocationAtTime(UAnimSequence* AnimationSequence, FName TargetBoneName, float Time)
{
	FTransform Transform;
	TArray<FName> BonePath;
	UAnimationBlueprintLibrary::FindBonePathToRoot(AnimationSequence, TargetBoneName, BonePath);
	for (auto& BoneName : BonePath)
	{
		FTransform BonePose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(AnimationSequence, BoneName, Time, false, BonePose);
		Transform *= BonePose;
	}

	return Transform.GetLocation();
}

void UAnimationModifier_BoneDistance::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	Super::OnApply_Implementation(AnimationSequence);

	FString Path = GetPathNameSafe(AnimationSequence);
	if (PathFilter == NAME_None || Path.Contains(PathFilter.ToString()))
	{
		for (const FBonePair& BonePair : BonePairs)
		{
			const FName& CurveName = *FString::Printf(TEXT("%s_to_%s"), *BonePair.BoneName1.ToString(), *BonePair.BoneName2.ToString());
			if (UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float))
			{
				UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName);
			}
			UAnimationBlueprintLibrary::AddCurve(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float, false);

			const float TotalTime = AnimationSequence->GetPlayLength();
			const int32 NumFrames = AnimationSequence->GetNumberOfFrames();
			const float TimePerFrame = TotalTime / NumFrames;

			for (int32 Frame = 0; Frame <= NumFrames; ++Frame)
			{
				float Time = AnimationSequence->GetTimeAtFrame(Frame);
				if (Frame == NumFrames)
				{
					// Subtract a little time from the last frame to avoid errors
					Time -= 0.001f;
				}

				const FVector Bone1Location = GetWorldBoneLocationAtTime(AnimationSequence, BonePair.BoneName1, Time);
				const FVector Bone2Location = GetWorldBoneLocationAtTime(AnimationSequence, BonePair.BoneName2, Time);
				const float BoneDistance = (Bone1Location - Bone2Location).Size() + BonePair.Offset;

				UAnimationBlueprintLibrary::AddFloatCurveKey(AnimationSequence, CurveName, Time, BoneDistance);
			}
		}

		UAnimationBlueprintLibrary::FinalizeBoneAnimation(AnimationSequence);
	}
}

void UAnimationModifier_BoneDistance::OnRevert_Implementation(UAnimSequence* AnimationSequence)
{
	Super::OnRevert_Implementation(AnimationSequence);

	for (const FBonePair& BonePair : BonePairs)
	{
		const FName& CurveName = *FString::Printf(TEXT("%s_to_%s"), *BonePair.BoneName1.ToString(), *BonePair.BoneName2.ToString());
		if (UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float))
		{
			UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName);
		}
	}
}
