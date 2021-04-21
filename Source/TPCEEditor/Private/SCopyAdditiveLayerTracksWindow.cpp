// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "SCopyAdditiveLayerTracksWindow.h"

#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "PropertyCustomizationHelpers.h"
#include "AssetThumbnail.h"

#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "Dialogs/Dialogs.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "CopyAdditiveLayerTracksWindow"

void SCopyAdditiveLayerTracksWindow::Construct(const FArguments& InArgs)
{
	WidgetWindow = InArgs._WidgetWindow;
	AnimSequences = InArgs._AnimSequences;

	// TODO thumbnails aren't working
	AssetThumbnailPool = MakeShareable(new FAssetThumbnailPool(16, false));

	// Find out if the selected animations share a single skeleton
	SkeletonPtr.Reset();
	for (auto AnimSequencePtr : InArgs._AnimSequences)
	{
		if (UAnimSequence* AnimSequence = AnimSequencePtr.Get())
		{
			if (!SkeletonPtr.IsValid())
			{
				SkeletonPtr = AnimSequence->GetSkeleton();
			}
			else if (SkeletonPtr != AnimSequence->GetSkeleton())
			{
				SkeletonPtr.Reset();
				break;
			}
		}
	}

	this->ChildSlot
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(3)
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(UAnimSequence::StaticClass())
				.OnObjectChanged(this, &SCopyAdditiveLayerTracksWindow::SetSourceAnimationSequence)
				.OnShouldFilterAsset(this, &SCopyAdditiveLayerTracksWindow::ShouldFilterAsset)
				.ObjectPath(this, &SCopyAdditiveLayerTracksWindow::GetSourceAnimationSequence)
				.ThumbnailPool(AssetThumbnailPool.ToSharedRef())
			]

			// TODO Will crash if the selected animation is open in Persona and the tracks are cleared
			// Not sure how to address this, so just warn the user for now
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(2)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CopyAdditiveLayerTracksWindow_Warning", "Please make sure Persona is closed before proceeding."))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(2)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(2)
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("CopyAdditiveLayerTracksWindow_Copy", "Copy"))
					.ToolTipText(LOCTEXT("CopyAdditiveLayerTracksWindow_Copy_ToolTip", "Copy additive layer tracks."))
					.IsEnabled(this, &SCopyAdditiveLayerTracksWindow::CanApply)
					.OnClicked(this, &SCopyAdditiveLayerTracksWindow::OnApply)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("CopyAdditiveLayerTracksWindow_Cancel", "Cancel"))
					.ToolTipText(LOCTEXT("CopyAdditiveLayerTracksWindow_Cancel_ToolTip", "Cancel copying additive layer tracks."))
					.OnClicked(this, &SCopyAdditiveLayerTracksWindow::OnCancel)
				]
			]
		]
	];
}

FReply SCopyAdditiveLayerTracksWindow::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		return OnCancel();
	}

	return FReply::Unhandled();
}

FReply SCopyAdditiveLayerTracksWindow::OnApply()
{
	const FScopedTransaction Transaction(LOCTEXT("UndoAction_CopyAdditiveLayerTracks", "Copying additive layer tracks to Animation Sequence(s)"));

	if (!SourceAnimSequence.IsValid() || !SourceAnimSequence->DoesContainTransformCurves())
	{
		static const FText MessageFormat = LOCTEXT("CopyAdditiveLayerTracksWindow_NoSourceTracks", "Additive layer tracks will be cleared, are you sure you want to proceed?");
		const bool bUserInputResult = OpenMsgDlgInt(EAppMsgType::YesNo, MessageFormat, LOCTEXT("CopyAdditiveLayerTracksWindow_NoSourceTracksTitle", "No source additive layer tracks")) == EAppReturnType::Yes;
		if (!bUserInputResult)
		{
			return FReply::Handled();
		}
	}

	for (auto AnimSequencePtr : AnimSequences)
	{
		CopyAdditiveLayerTracks(AnimSequencePtr.Get());
	}

	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}

	return FReply::Handled();
}

FReply SCopyAdditiveLayerTracksWindow::OnCancel()
{
	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}

	return FReply::Handled();
}

bool SCopyAdditiveLayerTracksWindow::CanApply() const
{
	return true;
}

void SCopyAdditiveLayerTracksWindow::SetSourceAnimationSequence(const FAssetData& InAssetData)
{
	SourceAnimSequence = Cast<UAnimSequence>(InAssetData.GetAsset());
}

FString SCopyAdditiveLayerTracksWindow::GetSourceAnimationSequence() const
{
	return SourceAnimSequence->GetPathName();
}

bool SCopyAdditiveLayerTracksWindow::ShouldFilterAsset(const FAssetData& InAssetData)
{
	if (InAssetData.GetClass() == UAnimSequence::StaticClass() && SkeletonPtr.IsValid())
	{
		FString SkeletonString = FAssetData(SkeletonPtr.Get()).GetExportTextName();
		FAssetDataTagMapSharedView::FFindTagResult Result = InAssetData.TagsAndValues.FindTag("Skeleton");
		return (!Result.IsSet() || SkeletonString != Result.GetValue());
	}

	return false;
}

void SCopyAdditiveLayerTracksWindow::CopyAdditiveLayerTracks(UAnimSequence* AnimSequence)
{
	if (!AnimSequence || SourceAnimSequence == AnimSequence)
	{
		return;
	}

	USkeleton* CurrentSkeleton = AnimSequence->GetSkeleton();
	check(CurrentSkeleton);

	if (AnimSequence->DoesContainTransformCurves())
	{
		AnimSequence->RawCurveData.DeleteAllCurveData(ERawCurveTrackTypes::RCT_Transform);
		AnimSequence->bNeedsRebake = true;
	}

	if (SourceAnimSequence.IsValid())
	{
		for (FTransformCurve SrcTransformCurve : SourceAnimSequence->RawCurveData.TransformCurves)
		{
			FName CurveName = SrcTransformCurve.Name.DisplayName;
			FSmartName NewCurveName;
			CurrentSkeleton->AddSmartNameAndModify(USkeleton::AnimTrackCurveMappingName, CurveName, NewCurveName);

			// Add curve - this won't add duplicate curve
			AnimSequence->RawCurveData.AddCurveData(NewCurveName, AACF_DriveTrack | AACF_Editable, ERawCurveTrackTypes::RCT_Transform);
			FTransformCurve* DstTransformCurve = static_cast<FTransformCurve*>(AnimSequence->RawCurveData.GetCurveData(NewCurveName.UID, ERawCurveTrackTypes::RCT_Transform));
			check(DstTransformCurve);

			DstTransformCurve->TranslationCurve.FloatCurves[0].SetKeys(SrcTransformCurve.TranslationCurve.FloatCurves[0].GetConstRefOfKeys());
			DstTransformCurve->TranslationCurve.FloatCurves[1].SetKeys(SrcTransformCurve.TranslationCurve.FloatCurves[1].GetConstRefOfKeys());
			DstTransformCurve->TranslationCurve.FloatCurves[2].SetKeys(SrcTransformCurve.TranslationCurve.FloatCurves[2].GetConstRefOfKeys());
			DstTransformCurve->RotationCurve.FloatCurves[0].SetKeys(SrcTransformCurve.RotationCurve.FloatCurves[0].GetConstRefOfKeys());
			DstTransformCurve->RotationCurve.FloatCurves[1].SetKeys(SrcTransformCurve.RotationCurve.FloatCurves[1].GetConstRefOfKeys());
			DstTransformCurve->RotationCurve.FloatCurves[2].SetKeys(SrcTransformCurve.RotationCurve.FloatCurves[2].GetConstRefOfKeys());
			DstTransformCurve->ScaleCurve.FloatCurves[0].SetKeys(SrcTransformCurve.ScaleCurve.FloatCurves[0].GetConstRefOfKeys());
			DstTransformCurve->ScaleCurve.FloatCurves[1].SetKeys(SrcTransformCurve.ScaleCurve.FloatCurves[1].GetConstRefOfKeys());
			DstTransformCurve->ScaleCurve.FloatCurves[2].SetKeys(SrcTransformCurve.ScaleCurve.FloatCurves[2].GetConstRefOfKeys());

			AnimSequence->bNeedsRebake = true;
		}
	}

	if (AnimSequence->DoesNeedRebake())
	{
		AnimSequence->Modify(true);
		AnimSequence->BakeTrackCurvesToRawAnimation();
	}

	if (AnimSequence->DoesNeedRecompress())
	{
		AnimSequence->Modify(true);
		AnimSequence->RequestSyncAnimRecompression(false);
	}
}

#undef LOCTEXT_NAMESPACE // "CopyAdditiveLayerTracksWindow"
