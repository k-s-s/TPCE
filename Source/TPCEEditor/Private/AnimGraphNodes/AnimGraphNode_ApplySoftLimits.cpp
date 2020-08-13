// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimGraphNodes/AnimGraphNode_ApplySoftLimits.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "TPCEAnimGraphNodes"

FText UAnimGraphNode_ApplySoftLimits::GetControllerDescription() const
{
	return LOCTEXT("ApplySoftLimits", "Apply Soft Limits");
}

FText UAnimGraphNode_ApplySoftLimits::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_ApplySoftLimits_Tooltip", "Apply Soft Limits.");
}

FText UAnimGraphNode_ApplySoftLimits::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return GetControllerDescription();
}

#undef LOCTEXT_NAMESPACE
