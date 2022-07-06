// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "Components/PushToTargetComponent.h"

#include "TopDownPushToTargetComponent.generated.h"

/**
 * A specialized PushToTargetComponent designed specifically for the TopDownPlayerController.
 */
UCLASS()
class TPCE_API UTopDownPushToTargetComponent : public UPushToTargetComponent
{
	GENERATED_BODY()

public:

	UTopDownPushToTargetComponent();

private:

	UPROPERTY(Transient, DuplicateTransient)
	class APlayerController* PlayerController;

	UPROPERTY(Transient, DuplicateTransient)
	TWeakObjectPtr<AActor> PreviousViewTarget;

	FVector PreviousViewTargetLocation;

protected:

	/**
	* Calculate an immediate adjustment to the updated component location, a headstart, along the view target's forward vector
	* and proportional to how it's aligned to the direction of movement.
	*
	* This produces the effect of dragging the updated component along with the view target depending on how straight the latter is moving.
	* The more sideways the view target moves (to a max of 90deg), the less will be the headstart (downto 0).
	*/
	virtual FVector AdjustCurrentLocationToTarget(const FVector& InCurrentLocation, const FVector& InTargetLocation) const override;

public:

	/**
	 * If true, adds an an immediate adjustment to the current location each tick, a headstart, that produces the effect of an increased attraction,
	 * proportional to how much the view target is aligned to its own direction of movement.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, meta = (editcondition = "bEnableTargetLag"))
	bool bAdjustTargetLagForViewTarget;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializeComponent() override;

};
