// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimationModifiers/AnimationModifier_FootSyncMarkers.h"
#include "Animation/AnimSequence.h"
#include "UObject/UObjectBaseUtility.h"
#include "ExtraTypes.h"
#include "Internationalization/Regex.h"

const FName UAnimationModifier_FootSyncMarkers::NotifyTrackName(TEXT("FootSync"));

FBoneModifier::FBoneModifier() :
	BoneName(NAME_None),
	Offset(0.0f)
{

}

FBoneModifier::FBoneModifier(FName InBoneName):
	BoneName(InBoneName),
	Offset(0.0f)
{
}

FBoneModifier::FBoneModifier(FName InBoneName, float InOffset):
	BoneName(InBoneName),
	Offset(InOffset)
{
}

UAnimationModifier_FootSyncMarkers::UAnimationModifier_FootSyncMarkers()
{
	PelvisBoneName = NAME_Root;
	bAxisFromName = true;
	MovementAxis = EAxisOption::Y;
	bMarkFootPlantOnly = true;

	FootBones.Add(FBoneModifier(NAME_Foot_L));
	FootBones.Add(FBoneModifier(NAME_Foot_R));
}

bool UAnimationModifier_FootSyncMarkers::CanEditChange(const UProperty* InProperty) const
{
	bool bCanChange = Super::CanEditChange(InProperty);

	if (bCanChange)
	{
		const FName PropertyName = (InProperty != NULL) ? InProperty->GetFName() : NAME_None;
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, CustomMovementAxis))
		{
			bCanChange = (MovementAxis == EAxisOption::Custom);
		}
	}

	return bCanChange;
}

void UAnimationModifier_FootSyncMarkers::RemoveSyncTrack(UAnimSequence* AnimationSequence)
{
	UAnimationBlueprintLibrary::RemoveAnimationNotifyTrack(AnimationSequence, NotifyTrackName);
	for (auto& FootBone : FootBones)
	{
		const FName CurveName = FootBone.BoneName;
		if (UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float))
			UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName, false);
	}
}

FVector UAnimationModifier_FootSyncMarkers::GetRelativeBoneLocationAtTime(UAnimSequence* AnimationSequence, FName TargetBoneName, FName RelativeBoneName, float Time)
{
	FTransform Transform;
	TArray<FName> BonePath;
	UAnimationBlueprintLibrary::FindBonePathToRoot(AnimationSequence, TargetBoneName, BonePath);
	for (auto& BoneName : BonePath)
	{
		if (BoneName == RelativeBoneName)
			break;

		FTransform BonePose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(AnimationSequence, BoneName, Time, false, BonePose);
		Transform *= BonePose;
	}

	return Transform.GetLocation();
}

void UAnimationModifier_FootSyncMarkers::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	Super::OnApply_Implementation(AnimationSequence);

	FString Path = GetPathNameSafe(AnimationSequence);
	if (PathFilter == NAME_None || Path.Contains(PathFilter.ToString()))
	{
		RemoveSyncTrack(AnimationSequence);
		UAnimationBlueprintLibrary::AddAnimationNotifyTrack(AnimationSequence, NotifyTrackName, FLinearColor::Green);

		FVector Axis = FVector::ZeroVector;
		if (bAxisFromName)
		{
			static const FRegexPattern AxisPattern(
				R"((Fwd)"				// Forward
				R"(|Bwd)"				// Backward
				R"(|[LR][A-Z\d][a-z])"	// Left/Right before another word (e.g A_RunLStart)
				R"(|[LR]\d*$)"			// Left/Right at end of sentence (e.g A_RunL2)
				R"(|\d{2,3}[LR]))"		// Angle and Left/Right (e.g A_Walk45R)
			);

			FRegexMatcher AxisMatcher(AxisPattern, AnimationSequence->GetName());
			if (AxisMatcher.FindNext())
			{
				FString Capture = AxisMatcher.GetCaptureGroup(1);
				if (Capture == TEXT("Fwd"))
				{
					Axis = FVector::RightVector;
				}
				else if (Capture == TEXT("Bwd"))
				{
					// Should be -FVector::RightVector, however most Bwd animations will face forward and -Y would invert the feet
					Axis = FVector::RightVector;
				}
				else if (Capture.StartsWith(TEXT("L")))
				{
					Axis = -FVector::ForwardVector;
				}
				else if (Capture.StartsWith(TEXT("R")))
				{
					Axis = FVector::ForwardVector;
				}
				else if (Capture.EndsWith(TEXT("L")))
				{
					const int32 Degrees = FCString::Atoi(*Capture.LeftChop(1));
					Axis = FRotator(0.0f, Degrees, 0.0f).Vector();
				}
				else if (Capture.EndsWith(TEXT("R")))
				{
					const int32 Degrees = FCString::Atoi(*Capture.LeftChop(1));
					Axis = FRotator(0.0f, -Degrees, 0.0f).Vector();
				}
			}
		}

		if (Axis.IsZero())
		{
			Axis = FAxisOption::GetAxisVector(MovementAxis, CustomMovementAxis.GetSafeNormal());
		}

		for (auto& FootBone : FootBones)
		{
			const FName& CurrentBoneName = FootBone.BoneName;
			const float CurrentBoneOffset = FootBone.Offset;

			const FName& CurveName = CurrentBoneName;			

			if (bCreateCurve)
			{
				if (UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float))
				{
					UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName);
				}
				UAnimationBlueprintLibrary::AddCurve(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float, false);
			}

			float LastBoneDistance = 0.0f;
			const float TotalTime = AnimationSequence->GetPlayLength();
			const int32 NumFrames = AnimationSequence->GetNumberOfFrames();
			const float TimePerFrame = TotalTime / NumFrames;
			for (int32 Frame = 0; Frame <= NumFrames; ++Frame)
			{
				// Subtract a little time from the last frame to avoid errors
				float Time = AnimationSequence->GetTimeAtFrame(Frame); 
				if (Frame == NumFrames)
					Time -= 0.001f;

				const FVector BoneRelativeLocation = GetRelativeBoneLocationAtTime(AnimationSequence, CurrentBoneName, PelvisBoneName, Time);
				const float BoneDistance = (BoneRelativeLocation * Axis).Size() * FMath::Sign(BoneRelativeLocation | Axis) + CurrentBoneOffset;

				if (bCreateCurve)
					UAnimationBlueprintLibrary::AddFloatCurveKey(AnimationSequence, CurveName, Time, BoneDistance);

				if (Frame > 0 && FMath::Sign(BoneDistance) != FMath::Sign(LastBoneDistance))
				{
					if (!bMarkFootPlantOnly || BoneDistance < 0.0f)
					{
						const FName MarkerName = *(CurrentBoneName.ToString().Append((BoneDistance >= 0.0f ? TEXT("_step_fwd") : bMarkFootPlantOnly ? TEXT("_plant") : TEXT("_step_bwd"))));
						const float MarkerTime = (BoneDistance == 0.0f) ? Time : (Time - (TimePerFrame * FMath::Abs(BoneDistance / (BoneDistance - LastBoneDistance))));
						UAnimationBlueprintLibrary::AddAnimationSyncMarker(AnimationSequence, MarkerName, MarkerTime, NotifyTrackName);
					}
				}

				LastBoneDistance = BoneDistance;
			}
		}

		UAnimationBlueprintLibrary::FinalizeBoneAnimation(AnimationSequence);
	}

}

void UAnimationModifier_FootSyncMarkers::OnRevert_Implementation(UAnimSequence* AnimationSequence)
{
	Super::OnRevert_Implementation(AnimationSequence);

	RemoveSyncTrack(AnimationSequence);

	if (bCreateCurve)
	{
		for (auto& FootBone : FootBones)
		{
			const FName& CurveName = FootBone.BoneName;
			if (UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float))
			{
				UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName);
			}
		}
	}
}

FName UAnimationModifier_FootSyncMarkers::GetSyncTrackName() const
{
	return NotifyTrackName;
}

