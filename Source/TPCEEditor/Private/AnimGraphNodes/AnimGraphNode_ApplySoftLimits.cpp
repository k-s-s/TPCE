// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimGraphNodes/AnimGraphNode_ApplySoftLimits.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

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

void UAnimGraphNode_ApplySoftLimits::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	Super::CustomizeDetails(DetailBuilder);

	TSharedRef<IPropertyHandle> NodeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAnimGraphNode_ApplySoftLimits, Node), GetClass());

	// if it doesn't have this node, that means this isn't really bone driven controller
	if (!NodeHandle->IsValidHandle())
	{
		return;
	}

	// Angular category
	IDetailCategoryBuilder& AngularCategory = DetailBuilder.EditCategory(TEXT("Angular"));

	AddTripletPropertyRow(
		/*Name=*/ LOCTEXT("FlipLabel", "Flip"),
		/*Tooltip=*/ LOCTEXT("FlipTooltip", "Flip limits"),
		/*Category=*/ AngularCategory,
		/*PropertyHandle=*/ NodeHandle,
		/*X=*/ GET_MEMBER_NAME_CHECKED(FAnimNode_ApplySoftLimits, bFlipX),
		/*Y=*/ GET_MEMBER_NAME_CHECKED(FAnimNode_ApplySoftLimits, bFlipY),
		/*Z=*/ GET_MEMBER_NAME_CHECKED(FAnimNode_ApplySoftLimits, bFlipZ),
		EVisibility::Visible);
}

void UAnimGraphNode_ApplySoftLimits::AddTripletPropertyRow(const FText& Name, const FText& Tooltip, IDetailCategoryBuilder& Category, TSharedRef<IPropertyHandle> PropertyHandle, const FName XPropertyName, const FName YPropertyName, const FName ZPropertyName, TAttribute<EVisibility> VisibilityAttribute)
{
	const float XYZPadding = 5.0f;

	TSharedPtr<IPropertyHandle> XProperty = PropertyHandle->GetChildHandle(XPropertyName);
	Category.GetParentLayout().HideProperty(XProperty);

	TSharedPtr<IPropertyHandle> YProperty = PropertyHandle->GetChildHandle(YPropertyName);
	Category.GetParentLayout().HideProperty(YProperty);

	TSharedPtr<IPropertyHandle> ZProperty = PropertyHandle->GetChildHandle(ZPropertyName);
	Category.GetParentLayout().HideProperty(ZProperty);

	Category.AddCustomRow(Name)
	.Visibility(VisibilityAttribute)
	.NameContent()
	[
		SNew(STextBlock)
		.Text(Name)
		.ToolTipText(Tooltip)
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(0.f, 0.f, XYZPadding, 0.f)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				XProperty->CreatePropertyNameWidget()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				XProperty->CreatePropertyValueWidget()
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(0.f, 0.f, XYZPadding, 0.f)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				YProperty->CreatePropertyNameWidget()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				YProperty->CreatePropertyValueWidget()
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(0.f, 0.f, XYZPadding, 0.f)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				ZProperty->CreatePropertyNameWidget()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				ZProperty->CreatePropertyValueWidget()
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
