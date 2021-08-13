// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimGraphNodes/AnimGraphNode_FootPlacement.h"
#include "AnimNodeEditModes.h"
#include "Animation/AnimInstance.h"
#include "UnrealWidget.h"

#define LOCTEXT_NAMESPACE "TPCEAnimGraphNodes"

UAnimGraphNode_FootPlacement::UAnimGraphNode_FootPlacement()
{

}


FText UAnimGraphNode_FootPlacement::GetTooltipText() const
{
	return LOCTEXT("FootPlacement", "Foot Placement");
}

FLinearColor UAnimGraphNode_FootPlacement::GetNodeTitleColor() const
{
	return FLinearColor(0.75f, 0.75f, 0.1f);
}

FText UAnimGraphNode_FootPlacement::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("FootPlacement", "Foot Placement");
}

void UAnimGraphNode_FootPlacement::CopyPinDefaultsToNodeData(UEdGraphPin * InPin)
{

}

#undef LOCTEXT_NAMESPACE