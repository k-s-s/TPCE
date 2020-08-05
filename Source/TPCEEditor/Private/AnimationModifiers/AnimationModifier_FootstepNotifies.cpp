// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimationModifiers/AnimationModifier_FootstepNotifies.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "UObject/UObjectBaseUtility.h"
#include "ReferenceSkeleton.h"
#include "ExtraTypes.h"

const FName UAnimationModifier_FootstepNotifies::NotifyTrackName(TEXT("Footsteps"));

UAnimationModifier_FootstepNotifies::UAnimationModifier_FootstepNotifies()
{
	FootDownThreshold = 1.0f;
	FootLiftThreshold = 2.0f;
	NudgeEvenThreshold = 0.06f;

	FootBoneNames.Add(NAME_Foot_L);
	FootBoneNames.Add(NAME_Foot_R);
}

FVector UAnimationModifier_FootstepNotifies::GetBoneWorldLocationAtTime(UAnimSequence* AnimationSequence, FName TargetBoneName, float Time)
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

FVector UAnimationModifier_FootstepNotifies::GetRefBoneWorldLocation(const FReferenceSkeleton& RefSkel, FName TargetBoneName)
{
	FTransform Transform;
	const int32 BoneIndex = RefSkel.FindBoneIndex(TargetBoneName);

	if (BoneIndex != INDEX_NONE)
	{
		Transform = RefSkel.GetRefBonePose()[BoneIndex];
		int32 ParentBoneIndex = RefSkel.GetRefBoneInfo()[BoneIndex].ParentIndex;

		while (ParentBoneIndex != INDEX_NONE)
		{
			Transform *= RefSkel.GetRefBonePose()[ParentBoneIndex];
			ParentBoneIndex = RefSkel.GetRefBoneInfo()[ParentBoneIndex].ParentIndex;
		}
	}

	return Transform.GetLocation();
}

void UAnimationModifier_FootstepNotifies::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	Super::OnApply_Implementation(AnimationSequence);

	FString Path = GetPathNameSafe(AnimationSequence);
	if (PathFilter != NAME_None && !Path.Contains(PathFilter.ToString()))
	{
		// Animation filtered, don't apply
		return;
	}

	const FReferenceSkeleton& RefSkel = AnimationSequence->GetSkeleton()->GetReferenceSkeleton();
	const float TotalTime = AnimationSequence->GetPlayLength();
	const int32 NumFrames = AnimationSequence->GetNumberOfFrames();
	const float TimePerFrame = TotalTime / NumFrames;

	UAnimationBlueprintLibrary::RemoveAnimationNotifyTrack(AnimationSequence, NotifyTrackName);
	UAnimationBlueprintLibrary::AddAnimationNotifyTrack(AnimationSequence, NotifyTrackName, FLinearColor::Green);
	const int32 NotifyTrackIndex = UAnimationBlueprintLibrary::GetTrackIndexForAnimationNotifyTrackName(AnimationSequence, NotifyTrackName);
	TArray<FAnimNotifyEvent*> Notifies;

	for (const FName& FootBoneName : FootBoneNames)
	{
		// Starting foot position
		const float FootHeight = GetRefBoneWorldLocation(RefSkel, FootBoneName).Z + KINDA_SMALL_NUMBER;
		const float StartTime = AnimationSequence->GetTimeAtFrame(0);
		const FVector StartBoneLocation = GetBoneWorldLocationAtTime(AnimationSequence, FootBoneName, StartTime);
		FVector PrevBoneDelta = FVector::ZeroVector;
		bool bPrevFootDown = StartBoneLocation.Z <= FootHeight + FootDownThreshold;

		for (int32 Frame = 1; Frame <= NumFrames; ++Frame)
		{
			const float PrevTime = AnimationSequence->GetTimeAtFrame(Frame - 1);
			float Time = AnimationSequence->GetTimeAtFrame(Frame);
			if (Frame == NumFrames)
			{
				// Subtract a little time from the last frame to avoid errors
				Time -= 0.001f;
			}

			const FVector PrevBoneLocation = GetBoneWorldLocationAtTime(AnimationSequence, FootBoneName, PrevTime);
			const FVector BoneLocation = GetBoneWorldLocationAtTime(AnimationSequence, FootBoneName, Time);
			const FVector BoneDelta = BoneLocation - PrevBoneLocation;
			bool bFootDown = BoneLocation.Z <= (bPrevFootDown ? FootHeight + FootLiftThreshold : FootHeight + FootDownThreshold);

			if (!bPrevFootDown && bFootDown)
			{
				FAnimNotifyEvent& NewEvent = AnimationSequence->Notifies.AddDefaulted_GetRef();
				NewEvent.NotifyName = FootBoneName;
				NewEvent.Link(AnimationSequence, Time);
				NewEvent.TriggerTimeOffset = GetTriggerTimeOffsetForType(AnimationSequence->CalculateOffsetForNotify(Time));
				NewEvent.TrackIndex = NotifyTrackIndex;
				NewEvent.NotifyStateClass = nullptr;
				Notifies.Add(&NewEvent);
			}

			bPrevFootDown = bFootDown;
			PrevBoneDelta = BoneDelta;
		}
	}

	if (NudgeEvenThreshold > 0.0f)
	{
		// Sort notifies by time
		Notifies.Sort([](FAnimNotifyEvent& A, FAnimNotifyEvent& B) { return A.GetTime() < B.GetTime(); });

		// Calculate the notify times as if they were evenly spaced. If the actual timing is close, nudge them towards the target values
		const int32 NumNotifies = Notifies.Num();
		float AverageTime = 0.0f;
		for (FAnimNotifyEvent* Event : Notifies)
		{
			AverageTime += Event->GetTime();
		}
		AverageTime /= NumNotifies;

		const float TimeBetweenNotifies = TotalTime / NumNotifies;
		const float StartTime = AverageTime + TimeBetweenNotifies / 2 - TotalTime / 2; // Can be negative

		bool bEvenlySpaced = true;
		for (int32 NotifyIndex = 0; NotifyIndex < NumNotifies; NotifyIndex++)
		{
			FAnimNotifyEvent* Event = Notifies[NotifyIndex];
			const float TargetTime = StartTime + TimeBetweenNotifies * NotifyIndex;

			if (FMath::Abs(TargetTime - Event->GetTime()) > NudgeEvenThreshold)
			{
				bEvenlySpaced = false;
				break;
			}
		}

		if (bEvenlySpaced)
		{
			// Determined that the intent is for the notifies to be evenly spaced, so go over them again
			for (int32 NotifyIndex = 0; NotifyIndex < NumNotifies; NotifyIndex++)
			{
				FAnimNotifyEvent* Event = Notifies[NotifyIndex];
				float TargetTime = StartTime + TimeBetweenNotifies * NotifyIndex;
				if (TargetTime < 0.0f)
				{
					TargetTime += TotalTime;
				}

				Event->SetTime(TargetTime);
				Event->TriggerTimeOffset = GetTriggerTimeOffsetForType(AnimationSequence->CalculateOffsetForNotify(TargetTime));
			}
		}
	}

	UAnimationBlueprintLibrary::FinalizeBoneAnimation(AnimationSequence);
}

void UAnimationModifier_FootstepNotifies::OnRevert_Implementation(UAnimSequence* AnimationSequence)
{
	Super::OnRevert_Implementation(AnimationSequence);

	UAnimationBlueprintLibrary::RemoveAnimationNotifyTrack(AnimationSequence, NotifyTrackName);
}
