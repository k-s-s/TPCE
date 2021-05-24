// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "BehaviorTree/BTService_ModifyFloat.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"

UBTService_ModifyFloat::UBTService_ModifyFloat(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Modify Float";

	bNotifyTick = true;
	bTickIntervals = true;

	// Accept only floats
	BlackboardKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ModifyFloat, BlackboardKey));
}

void UBTService_ModifyFloat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();

	if (MyBlackboard)
	{
		const float CurrentValue = MyBlackboard->GetValue<UBlackboardKeyType_Float>(BlackboardKey.GetSelectedKeyID());
		float NewValue = GetOperatorResult(Operator, CurrentValue, Operand);
		if (bClamp)
		{
			NewValue = FMath::Clamp(NewValue, Min, Max);
		}

		MyBlackboard->SetValue<UBlackboardKeyType_Float>(BlackboardKey.GetSelectedKeyID(), NewValue);
	}
}

FString UBTService_ModifyFloat::GetStaticDescription() const
{
	FString KeyDesc("invalid");
	if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
	{
		KeyDesc = BlackboardKey.SelectedKeyName.ToString();
	}

	return FString::Printf(TEXT("%s %s %.2f"), *KeyDesc, *GetOperatorDescription(Operator), Operand);
}

FString UBTService_ModifyFloat::GetOperatorDescription(EModifyFloatOperator Op) const
{
	switch (Op)
	{
	case EModifyFloatOperator::Assign:
		return "=";
	case EModifyFloatOperator::Add:
		return "+=";
	case EModifyFloatOperator::Subtract:
		return "-=";
	case EModifyFloatOperator::Multiply:
		return "*=";
	case EModifyFloatOperator::Divide:
		return "/=";
	}
	return "?";
}

template<typename T>
T UBTService_ModifyFloat::GetOperatorResult(EModifyFloatOperator Op, T Value1, T Value2) const
{
	switch (Op)
	{
	case EModifyFloatOperator::Assign:
		return Value2;
	case EModifyFloatOperator::Add:
		return Value1 + Value2;
	case EModifyFloatOperator::Subtract:
		return Value1 - Value2;
	case EModifyFloatOperator::Multiply:
		return Value1 * Value2;
	case EModifyFloatOperator::Divide:
		return (Value2 != 0.f) ? (Value1 / Value2) : 0.f;
	}
	return Value1;
}
