// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimGraphNodes/AnimGraphNode_OrientationWarping.h"
#include "AnimNodeEditModes.h"
#include "Animation/AnimInstance.h"
#include "UnrealWidget.h"

#define LOCTEXT_NAMESPACE "TPCEAnimGraphNodes"

UAnimGraphNode_OrientationWarping::UAnimGraphNode_OrientationWarping()
{

}

FLinearColor UAnimGraphNode_OrientationWarping::GetNodeTitleColor() const
{
	return FLinearColor(0.7f, 0.7f, 0.7f);
}

FText UAnimGraphNode_OrientationWarping::GetTooltipText() const
{
	return LOCTEXT("OrientationWarping", "Orientation Warping");
}

FText UAnimGraphNode_OrientationWarping::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("OrientationWarping", "Orientation Warping");
}

FString UAnimGraphNode_OrientationWarping::GetNodeCategory() const
{
	return TEXT("CustomTools");
}

#undef LOCTEXT_NAMESPACE

