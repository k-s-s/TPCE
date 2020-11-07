// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimationModifiers/AnimationModifier_RemoveBones.h"

#include "Animation/AnimSequence.h"
#include "UObject/UObjectBaseUtility.h"
#include "ExtraTypes.h"

UAnimationModifier_RemoveBones::UAnimationModifier_RemoveBones()
{
	bCaseInsensitive = true;
}

void UAnimationModifier_RemoveBones::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	Super::OnApply_Implementation(AnimationSequence);

	FString Path = GetPathNameSafe(AnimationSequence);
	if (PathFilter != NAME_None && !Path.Contains(PathFilter.ToString()))
	{
		return;
	}

	// Copy bone filters with or without case and remove the '+' prefix
	TArray<FWildcardString> BoneWildcards;
	const int32 NumFilters = BoneFilters.Num();
	BoneWildcards.Reserve(NumFilters);
	for (const FString& BoneFilter : BoneFilters)
	{
		const FString NoPrefix = BoneFilter.StartsWith(TEXT("+")) ? BoneFilter.RightChop(1) : BoneFilter;
		BoneWildcards.Add(bCaseInsensitive ? NoPrefix.ToLower() : NoPrefix);
	}

	// Find indices of all the bones that match the filters
	TMap<int32, bool> MatchingBones;
	USkeleton* Skeleton = AnimationSequence->GetSkeleton();
	if (Skeleton)
	{
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

		// Bones are in strictly increasing order
		// Adding a bone to MatchingBones with True will knock out the entire branch
		const int32 NumBones = RefSkeleton.GetNum();

		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			const int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);
			if (bool* bWithChildren = MatchingBones.Find(ParentBoneIndex))
			{
				if (*bWithChildren)
				{
					// Parent bone matched and included children, so add this and include subsequent children
					MatchingBones.Add(BoneIndex, true);
					continue;
				}
			}

			FString BoneName = RefSkeleton.GetBoneName(BoneIndex).ToString();
			if (bCaseInsensitive)
			{
				BoneName.ToLowerInline();
			}

			for (int32 FilterIndex = 0; FilterIndex < NumFilters; FilterIndex++)
			{
				if (BoneWildcards[FilterIndex].IsMatch(BoneName))
				{
					bool bIncludesChildren = BoneFilters[FilterIndex].StartsWith(TEXT("+"));
					MatchingBones.Add(BoneIndex, bIncludesChildren);
					if (bIncludesChildren)
					{
						// No need to keep matching
						break;
					}
				}
			}
		}

		// Can't call UAnimSequence::RemoveTrack directly, so unfortunately UAnimationBlueprintLibrary has to be relied on
		// Keep in mind GetAnimationTrackNames returns the actual array, so we must iterate backwards while removing
		const TArray<FName>& TrackNames = AnimationSequence->GetAnimationTrackNames();
		for (int32 TrackIndex = TrackNames.Num() - 1; TrackIndex >= 0; --TrackIndex)
		{
			const FName TrackName = TrackNames[TrackIndex];
			const int32 BoneIndex = RefSkeleton.FindBoneIndex(TrackName);
			if (MatchingBones.Contains(BoneIndex) != bInvertSelection)
			{
				UAnimationBlueprintLibrary::RemoveBoneAnimation(AnimationSequence, TrackName, /*bIncludeChildren=*/false, /*bFinalize=*/false);
			}
		}

		// Copy curve filters with or without case
		TArray<FWildcardString> CurveWildcards;
		CurveWildcards.Reserve(CurveFilters.Num());
		for (const FString& CurveFilter : CurveFilters)
		{
			CurveWildcards.Add(bCaseInsensitive ? CurveFilter.ToLower() : CurveFilter);
		}

		// Collect names of curves to be removed
		TArray<FName> CurveNamesToRemove;
		for (const FFloatCurve& Curve : AnimationSequence->RawCurveData.FloatCurves)
		{
			FString CurveName = Curve.Name.DisplayName.ToString();
			if (bCaseInsensitive)
			{
				CurveName.ToLowerInline();
			}

			for (const FWildcardString& Wildcard : CurveWildcards)
			{
				if (Wildcard.IsMatch(CurveName))
				{
					CurveNamesToRemove.Add(Curve.Name.DisplayName);
					break;
				}
			}
		}

		for (const FName& CurveName : CurveNamesToRemove)
		{
			UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName);
		}

		UAnimationBlueprintLibrary::FinalizeBoneAnimation(AnimationSequence);
	}
}
