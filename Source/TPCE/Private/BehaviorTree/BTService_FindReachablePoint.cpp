// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "BehaviorTree/BTService_FindReachablePoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "NavigationSystem.h"

UBTService_FindReachablePoint::UBTService_FindReachablePoint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Find Reachable Point";

	bNotifyTick = false;
	bTickIntervals = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	Radius = 100.f;

	// Accept only actors and vectors
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FindReachablePoint, BlackboardKey), AActor::StaticClass());
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FindReachablePoint, BlackboardKey));
	Origin.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FindReachablePoint, Origin), AActor::StaticClass());
	Origin.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FindReachablePoint, Origin));
	Origin.AllowNoneAsValue(true);
}

void UBTService_FindReachablePoint::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		Origin.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_FindReachablePoint::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	const AAIController* MyController = OwnerComp.GetAIOwner();
	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();

	if (MyController && MyBlackboard)
	{
		bool bValidOrigin = false;
		FVector OriginLocation;
		const APawn* ControlledPawn = MyController->GetPawn();

		if (Origin.IsSet())
		{
			bValidOrigin = MyBlackboard->GetLocationFromEntry(Origin.GetSelectedKeyID(), OriginLocation);
		}
		else if (ControlledPawn)
		{
			OriginLocation = ControlledPawn->GetActorLocation();
			bValidOrigin = true;
		}

		if (bValidOrigin)
		{
			if (ControlledPawn)
			{
				OriginLocation += MyController->GetPawn()->GetActorQuat().RotateVector(OriginOffset);
			}

			FVector Result;
			if (!GetRandomReachablePoint(OwnerComp, OriginLocation, Result))
			{
				// Couldn't find a valid reachable point
				// An invalid location will cause a MoveTo task to fail, so return the origin instead
				Result = OriginLocation;
			}
			MyBlackboard->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), Result);
		}
		else
		{
			MyBlackboard->ClearValue(BlackboardKey.GetSelectedKeyID());
		}
	}
}

bool UBTService_FindReachablePoint::GetRandomReachablePoint(const UBehaviorTreeComponent& OwnerComp, const FVector& InOrigin, FVector& OutRandomPoint) const
{
	if (const AAIController* MyController = OwnerComp.GetAIOwner())
	{
		if (UWorld* MyWorld = MyController->GetWorld())
		{
			if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(MyWorld))
			{
				if (ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate))
				{
					TSubclassOf<UNavigationQueryFilter> UseFilterClass = *FilterClass ? FilterClass : MyController->GetDefaultNavigationFilterClass();
					FNavLocation RandomPoint(InOrigin);
					if (NavSys->GetRandomReachablePointInRadius(InOrigin, Radius, RandomPoint, NavData,
						UNavigationQueryFilter::GetQueryFilter(*NavData, MyController, UseFilterClass)))
					{
						OutRandomPoint = RandomPoint.Location;
						return true;
					}
				}
			}
		}
	}
	return false;
}

FString UBTService_FindReachablePoint::GetStaticDescription() const
{
	FString KeyDesc("invalid");
	if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() ||
		BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		KeyDesc = BlackboardKey.SelectedKeyName.ToString();
	}

	FString OriginDesc("self");
	if (Origin.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() ||
		Origin.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		OriginDesc = Origin.SelectedKeyName.ToString();
	}

	return FString::Printf(TEXT("Set %s %.2f units from %s"), *KeyDesc, Radius, *OriginDesc);
}

#if WITH_EDITOR

FName UBTService_FindReachablePoint::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Service.DefaultFocus.Icon");
}

#endif // WITH_EDITOR
