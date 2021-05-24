// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "InputCoreTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_Chance.generated.h"

class UBehaviorTree;
class UBlackboardComponent;

/**
 * Random chance decorator node.
 */
UCLASS()
class TPCE_API UBTDecorator_Chance : public UBTDecorator
{
	GENERATED_UCLASS_BODY()

	/** Chance percent for the condition to pass. */
	UPROPERTY(EditAnywhere, Category="Condition", meta=(ClampMin="0", UIMin="0", ClampMax="100", UIMax="100"))
	float Chance;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR

protected:

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
