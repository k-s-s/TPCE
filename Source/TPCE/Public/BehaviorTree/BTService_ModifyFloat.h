// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"

#include "BTService_ModifyFloat.generated.h"

/** Enum specifying operator type. */
UENUM()
enum class EModifyFloatOperator : uint8
{
	/** Assignment operator. */
	Assign,
	/** Addition operator. */
	Add,
	/** Subtraction operator. */
	Subtract,
	/** Multiplication operator. */
	Multiply,
	/** Division operator. */
	Divide,
};

/**
 * Modifies a float blackboard value.
 */
UCLASS()
class TPCE_API UBTService_ModifyFloat : public UBTService_BlackboardBase
{
	GENERATED_UCLASS_BODY()

	/** Type of operation. */
	UPROPERTY(EditAnywhere, Category="Node")
	EModifyFloatOperator Operator;

	/** Second operand. */
	UPROPERTY(EditAnywhere, Category="Node")
	float Operand;

	/** Keep value within the allowed range. */
	UPROPERTY(EditAnywhere, Category="Node")
	bool bClamp;

	/** Minimum allowed value. */
	UPROPERTY(EditAnywhere, Category="Node", meta=(EditCondition="bClamp"))
	float Min;

	/** Maximum allowed value. */
	UPROPERTY(EditAnywhere, Category="Node", meta=(EditCondition="bClamp"))
	float Max;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:

	FString GetOperatorDescription(EModifyFloatOperator Op) const;

	template<typename T>
	T GetOperatorResult(EModifyFloatOperator Op, T Value1, T Value2) const;
};
