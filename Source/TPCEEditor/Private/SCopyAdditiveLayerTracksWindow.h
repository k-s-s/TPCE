// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"

#include "Templates/SharedPointer.h"
#include "Containers/Array.h"

class UAnimSequence;
class USkeleton;
class FAssetThumbnailPool;
struct FAssetData;

/** UI slate widget allowing the user to copy additive layer tracks to a selection of Animation Sequences. */
class SCopyAdditiveLayerTracksWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCopyAdditiveLayerTracksWindow)
		: _WidgetWindow()
	{}
		SLATE_ARGUMENT(TSharedPtr<SWindow>, WidgetWindow)
		SLATE_ARGUMENT(TArray<TWeakObjectPtr<UAnimSequence>>, AnimSequences)
	SLATE_END_ARGS()

public:

	SCopyAdditiveLayerTracksWindow() {}
	void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

protected:

	FReply OnApply();
	FReply OnCancel();
	bool CanApply() const;

	void SetSourceAnimationSequence(const FAssetData& InAssetData);
	FString GetSourceAnimationSequence() const;
	bool ShouldFilterAsset(const FAssetData& InAssetData);

private:

	TWeakPtr<SWindow> WidgetWindow;
	TArray<TWeakObjectPtr<UAnimSequence>> AnimSequences;
	TSharedPtr<FAssetThumbnailPool> AssetThumbnailPool;

	TWeakObjectPtr<USkeleton> SkeletonPtr;
	TWeakObjectPtr<UAnimSequence> SourceAnimSequence;

	void CopyAdditiveLayerTracks(UAnimSequence* DestAnimSequence);
};
