// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Components/ExtPathFollowingComponent.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "AIConfig.h"

UExtPathFollowingComponent::UExtPathFollowingComponent()
{
}

FVector UExtPathFollowingComponent::GetMoveFocus(bool bAllowStrafe) const
{
	// Similar to the original implementation, with more natural look targets
	// - Asks DestinationActor for its target location, allowing it to specialcase or otherwise return a point that isn't always the feet
	// - When looking into the distance, looks at a point at eye height

	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (!MyPawn)
	{
		if (AController* MyController = Cast<AController>(GetOwner()))
		{
			MyPawn = MyController->GetPawn();
		}
	}

	FVector MoveFocus = FVector::ZeroVector;
	if (bAllowStrafe && DestinationActor.IsValid())
	{
		MoveFocus = DestinationActor->GetTargetLocation(MyPawn);
	}
	else
	{
		const FVector CurrentMoveDirection = GetCurrentDirection();
		MoveFocus = *CurrentDestination + (CurrentMoveDirection * FAIConfig::Navigation::FocalPointDistance);
		if (MyPawn)
		{
			MoveFocus.Z = MyPawn->GetActorLocation().Z + MyPawn->BaseEyeHeight;
		}
	}

	return MoveFocus;
}
