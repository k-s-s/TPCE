// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "GameFramework/TopDownPushToTargetComponent.h"

#include "GameFramework/PlayerController.h"

UTopDownPushToTargetComponent::UTopDownPushToTargetComponent()
{
	bWantsInitializeComponent = true;
	bAdjustTargetLagForViewTarget = false;
}

FVector UTopDownPushToTargetComponent::AdjustCurrentLocationToTarget(const FVector& InCurrentLocation, const FVector& InTargetLocation) const
{
	if (bAdjustTargetLagForViewTarget)
	{
		if (const AActor* const ViewTarget = PreviousViewTarget.Get())
		{
			const FVector HeadStart = (ViewTarget->GetActorLocation() - PreviousViewTargetLocation).ProjectOnTo(ViewTarget->GetActorForwardVector());
			// If the head start we're applying to our updated component is not opposing to where we want it to be, we can
			if (FVector::DotProduct(HeadStart, InTargetLocation - InCurrentLocation) > 0.0f)
				return InCurrentLocation + HeadStart;
		}
	}

	return InCurrentLocation;
}

void UTopDownPushToTargetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	check(PlayerController);

	AActor* ViewTarget = PlayerController->GetViewTarget();
	if (PreviousViewTarget != ViewTarget)
	{
		PreviousViewTarget = ViewTarget;
		PreviousViewTargetLocation = FVector::ZeroVector;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ViewTarget)
		PreviousViewTargetLocation = ViewTarget->GetActorLocation();
}

void UTopDownPushToTargetComponent::InitializeComponent()
{
	Super::InitializeComponent();

	PlayerController = CastChecked<APlayerController>(GetOwner());
}


