// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimGraphNodes/AnimGraphNode_ArmSeparation.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "TPCEAnimGraphNodes"

FText UAnimGraphNode_ArmSeparation::GetControllerDescription() const
{
	return LOCTEXT("ArmSeparation", "Arm Separation");
}

FText UAnimGraphNode_ArmSeparation::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_ArmSeparation_Tooltip", "Pushes a bone away from a collider shape by pivoting another joint. "
		"Useful to prevent hands from intersecting thighs caused by retargeting between different body types.");
}

FText UAnimGraphNode_ArmSeparation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return GetControllerDescription();
}

void UAnimGraphNode_ArmSeparation::Draw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* SkelMeshComp) const
{
	if (SkelMeshComp)
	{
		if (FAnimNode_ArmSeparation* ActiveNode = GetActiveInstanceNode<FAnimNode_ArmSeparation>(SkelMeshComp->GetAnimInstance()))
		{
			ActiveNode->ConditionalDebugDraw(PDI, SkelMeshComp);
		}
	}
}

void UAnimGraphNode_ArmSeparation::GetOnScreenDebugInfo(TArray<FText>& DebugInfo, FAnimNode_Base* RuntimeAnimNode, USkeletalMeshComponent* PreviewSkelMeshComp) const
{
	if (RuntimeAnimNode)
	{
		const FAnimNode_ArmSeparation* ActiveNode = static_cast<FAnimNode_ArmSeparation*>(RuntimeAnimNode);
		DebugInfo.Add(FText::Format(LOCTEXT("DebugOnScreenBoneName", "Anim Arm Separation (Source:{0})"), FText::FromName(ActiveNode->PivotBone.BoneName)));
		DebugInfo.Add(FText::Format(LOCTEXT("DebugOnScreenAlpha", "	Current Alpha: {0}"), ActiveNode->CachedDisplacementAlpha));
	}
}

#undef LOCTEXT_NAMESPACE
