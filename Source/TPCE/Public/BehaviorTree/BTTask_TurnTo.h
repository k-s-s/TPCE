// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "BTTask_TurnTo.generated.h"

/**
 * Turn To task node.
 * Rotates the AI pawn to face the specified Actor or Location blackboard entry.
 */
UCLASS()
class TPCE_API UBTTask_TurnTo : public UBTTask_BlackboardBase
{
	GENERATED_UCLASS_BODY()

	/** Rotation speed, higher is faster. Specified in degrees per second if bConstantSpeed is set. */
	UPROPERTY(EditAnywhere, Category="Node", meta=(ClampMin="0", UIMin="0"))
	float Speed;

	/** If set and the controlled pawn is a character, scale speed by the current value of a curve. */
	UPROPERTY(EditAnywhere, Category="Node")
	FName SpeedCurveName;

	/** Stop when the angle difference is less than Tolerance degrees. Continues indefinitely if 0. */
	UPROPERTY(EditAnywhere, Category="Node", meta=(ClampMin="0", UIMin="0"))
	float Tolerance;

	/** If True, rotation speed is constant instead of starting strong then easing out. */
	UPROPERTY(EditAnywhere, Category="Node")
	bool bConstantSpeed;

	/** Rotate only on the ground plane. */
	UPROPERTY(EditAnywhere, Category="Node")
	bool bYawOnly;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR

protected:

	FQuat GetLookAtRotation(const FVector& StartLocation, const FVector& TargetLocation) const;
};
