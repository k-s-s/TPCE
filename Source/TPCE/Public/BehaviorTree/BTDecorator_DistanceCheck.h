// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "AITypes.h"

#include "BTDecorator_DistanceCheck.generated.h"

struct FBTDistanceCheckDecoratorMemory
{
	bool bLastRawResult;
};

/**
 * Distance check decorator node.
 * A decorator node that bases its condition on a distance check, using Blackboard entries to form the parameters of the check.
 */
UCLASS()
class TPCE_API UBTDecorator_DistanceCheck : public UBTDecorator
{
	GENERATED_UCLASS_BODY()

	typedef FBTDistanceCheckDecoratorMemory TNodeInstanceMemory;

	/** Minimum distance required. Always passes if 0. */
	UPROPERTY(EditAnywhere, Category="Condition", meta=(ClampMin="0", UIMin="0"))
	float MinDistance;

	/** Maximum distance required. Always passes if 0. */
	UPROPERTY(EditAnywhere, Category="Condition", meta=(ClampMin="0", UIMin="0"))
	float MaxDistance;

	UPROPERTY(EditAnywhere, Category="Condition")
	FAIDistanceType GeometricDistanceType;

	/** Blackboard key selector */
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector Observed;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(TNodeInstanceMemory); }
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;
	virtual FString GetStaticDescription() const override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR

protected:

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	void OnBlackboardChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:

	float MinDistanceSqr;
	float MaxDistanceSqr;

	bool CalcConditionImpl(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;
	float GetGeometricDistanceSquared(const FVector& A, const FVector& B) const;
	bool CalculateDistance(const UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& Target, float& Distance) const;
};
