// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimGraphNodes/AnimGraphNode_SpeedWarping.h"
#include "AnimNodeEditModes.h"
#include "Animation/AnimInstance.h"
#include "UnrealWidget.h"
#include "Components/SkeletalMeshComponent.h"

#define LOCTEXT_NAMESPACE "TPCEAnimGraphNodes"

UAnimGraphNode_SpeedWarping::UAnimGraphNode_SpeedWarping()
{

}


FText UAnimGraphNode_SpeedWarping::GetTooltipText() const
{
	return LOCTEXT("SpeedWarping", "Speed Warping");
}

FLinearColor UAnimGraphNode_SpeedWarping::GetNodeTitleColor() const
{
	return FLinearColor(0.75f, 0.75f, 0.1f);
}

FText UAnimGraphNode_SpeedWarping::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("SpeedWarping", "Speed Warping");
}

void UAnimGraphNode_SpeedWarping::CopyPinDefaultsToNodeData(UEdGraphPin * InPin)
{

}

void UAnimGraphNode_SpeedWarping::Draw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelMeshComp) const
{
	if (bEnableDebugDraw && SkelMeshComp)
	{
		if (FAnimNode_SpeedWarping* ActiveNode = GetActiveInstanceNode<FAnimNode_SpeedWarping>(SkelMeshComp->GetAnimInstance()))
		{
			ActiveNode->ConditionalDebugDraw(PDI, SkelMeshComp);
		}
	}
}

#undef LOCTEXT_NAMESPACE