// Copyright (c) 2020 greisane <ggreisane@gmail.com>

#include "GameFramework/FloaterActor.h"

#include "Components/PushToTargetComponent.h"
#include "Curves/CurveLinearColor.h"
#include "UObject/ConstructorHelpers.h"

FName AFloaterActor::MovementComponentName(TEXT("Movement"));

AFloaterActor::AFloaterActor()
	: AnimationRate(1.0f)
	, ShowState(EFloaterShowState::Shown)
{
	static ConstructorHelpers::FObjectFinder<UCurveLinearColor> PopUpCurveObject(TEXT("/TPCE/UI/C4_PopUpBottom.C4_PopUpBottom"));
	static ConstructorHelpers::FObjectFinder<UCurveLinearColor> PopInCurveObject(TEXT("/TPCE/UI/C4_PopInBottom.C4_PopInBottom"));
	PopUpCurve = PopUpCurveObject.Object;
	PopInCurve = PopInCurveObject.Object;

	MovementComponent = CreateDefaultSubobject<UPushToTargetComponent>(MovementComponentName);
	MovementComponent->RelativeOffset = FVector(0.0f, 80.0f, 0.0f);
	MovementComponent->bEnableLag = true;
	MovementComponent->Speed = 3.0f;
	MovementComponent->RotationSpeed = 2.8f;
	MovementComponent->RotationType = EPushToTargetRotationType::FaceView;
	MovementComponent->bStayUpright = false;
	MovementComponent->bForceSubStepping = false;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bTickEvenWhenPaused = false;
}

void AFloaterActor::BeginPlay()
{
	if (PopUpCurve && PopInCurve)
	{
		FOnTimelineLinearColorStatic OnTimelineProgressDelegate;
		OnTimelineProgressDelegate.BindUObject(this, &AFloaterActor::OnTimelineProgress);
		FOnTimelineEventStatic OnTimelineFinishedDelegate;
		OnTimelineFinishedDelegate.BindUObject(this, &AFloaterActor::OnTimelineFinished);
		float MinTime, MaxTime;

		PopUpTimeline.AddInterpLinearColor(PopUpCurve, OnTimelineProgressDelegate);
		PopUpCurve->GetTimeRange(MinTime, MaxTime);
		PopUpTimeline.SetTimelineLength(MaxTime);
		PopUpTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);
		PopUpTimeline.SetTimelineFinishedFunc(OnTimelineFinishedDelegate);
		PopUpTimeline.SetLooping(false);

		PopInTimeline.AddInterpLinearColor(PopInCurve, OnTimelineProgressDelegate);
		PopInCurve->GetTimeRange(MinTime, MaxTime);
		PopInTimeline.SetTimelineLength(MaxTime);
		PopInTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);
		PopInTimeline.SetTimelineFinishedFunc(OnTimelineFinishedDelegate);
		PopInTimeline.SetLooping(false);
	}
	
	ShowState = IsHidden() ? EFloaterShowState::Hidden : EFloaterShowState::Shown;
	if (ShowState == EFloaterShowState::Shown)
	{
		SetActorTickEnabled(true);
	}

	Super::BeginPlay();
}

void AFloaterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update scale
	PopUpTimeline.TickTimeline(DeltaTime);
	PopInTimeline.TickTimeline(DeltaTime);
}

void AFloaterActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (bNewHidden && ShowState != EFloaterShowState::Hidden)
	{
		PopUpTimeline.Stop();
		PopInTimeline.Stop();
		ShowState = EFloaterShowState::Hidden;
	}
	else if (!bNewHidden && ShowState != EFloaterShowState::Showing)
	{
		Show();
	}
}

void AFloaterActor::Show()
{
	if (ShowState == EFloaterShowState::Hidden)
	{
		// Begin playing, currently invisible so snap to the desired transform
		MovementComponent->SnapToTarget();

		SetActorRelativeScale3D(FVector::ZeroVector);

		ShowState = EFloaterShowState::Showing;
		PopUpTimeline.SetPlayRate(AnimationRate);
		PopUpTimeline.PlayFromStart();

		SetActorTickEnabled(true);
		SetActorHiddenInGame(false);
	}
	else if (ShowState == EFloaterShowState::Hiding)
	{
		// Undo current progress
		ShowState = EFloaterShowState::Showing;
		PopInTimeline.SetPlayRate(AnimationRate);
		PopInTimeline.Reverse();
	}
}

void AFloaterActor::Hide()
{
	if (ShowState == EFloaterShowState::Shown)
	{
		// Resume playing the rest of the curve
		ShowState = EFloaterShowState::Hiding;
		PopInTimeline.SetPlayRate(AnimationRate);
		PopInTimeline.PlayFromStart();
	}
	else if (ShowState == EFloaterShowState::Showing)
	{
		// Undo current progress
		ShowState = EFloaterShowState::Hiding;
		PopUpTimeline.SetPlayRate(AnimationRate);
		PopUpTimeline.Reverse();
	}
}

void AFloaterActor::OnTimelineProgress(FLinearColor Value)
{
	SetActorRelativeScale3D(FVector(Value));
}

void AFloaterActor::OnTimelineFinished()
{
	if (ShowState == EFloaterShowState::Showing)
	{
		ShowState = EFloaterShowState::Shown;
	}
	else if (ShowState == EFloaterShowState::Hiding)
	{
		// Finished hiding
		ShowState = EFloaterShowState::Hidden;

		SetActorTickEnabled(false);
		SetActorHiddenInGame(true);
	}
}
