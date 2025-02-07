// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"

#include "BTService_Distance.generated.h"

/**
 * Distance service node.
 * Finds the distance between the controlled pawn and the observed target and assigns it to a Blackboard entry.
 */
UCLASS()
class TPCE_API UBTService_Distance : public UBTService_BlackboardBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category="Node")
	FAIDistanceType GeometricDistanceType;

	/** If set, AI pawn's capsule dimensions will be subtracted from the result.  */
	UPROPERTY(EditAnywhere, Category="Node")
	bool bReachTestIncludesAgentRadius;

	/** If set, observed pawn's capsule dimensions will be subtracted from the result. */
	UPROPERTY(EditAnywhere, Category="Node")
	bool bReachTestIncludesGoalRadius;

	/** Blackboard key selector */
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector Observed;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR

protected:

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:

	float GetGeometricDistance(const FVector& A, const FVector& B, float RadiusSum, float HalfHeightSum) const;
	static FString GetGeometricDistanceDescription(FAIDistanceType GeometricDistanceType);
};
