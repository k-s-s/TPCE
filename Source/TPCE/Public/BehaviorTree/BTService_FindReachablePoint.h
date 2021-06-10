// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"

#include "BTService_FindReachablePoint.generated.h"

/**
 * Find Reachable Point service node.
 * A service node that finds a navigation reachable point from the initial location when it becomes active.
 */
UCLASS()
class TPCE_API UBTService_FindReachablePoint : public UBTService_BlackboardBase
{
	GENERATED_UCLASS_BODY()

	/** Initial search location. "None" will use the pawn's current location. */
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector Origin;

	/** Search radius. */
	UPROPERTY(EditAnywhere, Category="Node", meta=(ClampMin="0", UIMin="0"))
	float Radius;

	/** "None" will result in default filter being used. */
	UPROPERTY(EditAnywhere, Category="Node")
	TSubclassOf<UNavigationQueryFilter> FilterClass;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual FString GetStaticDescription() const override;

	EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR
};
