// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "BehaviorTree/BTDecorator_Chance.h"

#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_Chance::UBTDecorator_Chance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Chance";
}

bool UBTDecorator_Chance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return FMath::FRand() * 100.f < Chance;
}

#if WITH_EDITOR

FName UBTDecorator_Chance::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Decorator.Conditional.Icon");
}

#endif	// WITH_EDITOR
