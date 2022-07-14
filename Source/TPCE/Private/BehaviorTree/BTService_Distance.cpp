// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "BehaviorTree/BTService_Distance.h"

#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "AIController.h"

UBTService_Distance::UBTService_Distance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Distance";
	bNotifyTick = true;
	bTickIntervals = true;

	GeometricDistanceType = FAIDistanceType::Distance3D;

	// Accept only floats
	BlackboardKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, BlackboardKey));

	// Accept only actors and vectors
	Observed.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Observed), AActor::StaticClass());
	Observed.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Observed));
}

void UBTService_Distance::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		Observed.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_Distance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	FVector TargetLocation = FVector::ZeroVector;
	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	const AAIController* MyController = OwnerComp.GetAIOwner();

	if (MyBlackboard && MyBlackboard->GetLocationFromEntry(Observed.GetSelectedKeyID(), TargetLocation) && MyController && MyController->GetPawn())
	{
		float CollisionRadiusSum = 0.f;
		float CollisionHalfHeightSum = 0.f;

		if (bReachTestIncludesAgentRadius)
		{
			// Could be cached
			MyController->GetPawn()->GetSimpleCollisionCylinder(CollisionRadiusSum, CollisionHalfHeightSum);
		}

		if (bReachTestIncludesGoalRadius)
		{
			// Could be cached?
			UObject* ObservedValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(Observed.GetSelectedKeyID());
			if (AActor* ObservedPawn = Cast<APawn>(ObservedValue))
			{
				float CollisionRadius, CollisionHalfHeight;
				ObservedPawn->GetSimpleCollisionCylinder(CollisionRadius, CollisionHalfHeight);

				CollisionRadiusSum += CollisionRadius;
				CollisionHalfHeightSum += CollisionHalfHeight;
			}
		}

		const float Distance = GetGeometricDistance(MyController->GetPawn()->GetActorLocation(), TargetLocation, CollisionRadiusSum, CollisionHalfHeightSum);

		MyBlackboard->SetValue<UBlackboardKeyType_Float>(BlackboardKey.GetSelectedKeyID(), Distance);
	}
}

FString UBTService_Distance::GetStaticDescription() const
{
	FString KeyDesc("invalid");
	if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
	{
		KeyDesc = BlackboardKey.SelectedKeyName.ToString();
	}

	FString ObservedKeyDesc("invalid");
	if (Observed.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() ||
		Observed.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		ObservedKeyDesc = Observed.SelectedKeyName.ToString();
	}

	return FString::Printf(TEXT("%s\n%s = %s to %s"), *Super::GetStaticDescription(), *KeyDesc, *GetGeometricDistanceDescription(GeometricDistanceType), *ObservedKeyDesc);
}

float UBTService_Distance::GetGeometricDistance(const FVector& A, const FVector& B, float RadiusSum, float HalfHeightSum) const
{
	switch (GeometricDistanceType)
	{
	case FAIDistanceType::Distance3D:
		return FMath::Max(0.f, FVector::Dist(A, B) - RadiusSum);  // Wrong but whatever
	case FAIDistanceType::Distance2D:
		return FMath::Max(0.f, FVector::DistXY(A, B) - RadiusSum);
	case FAIDistanceType::DistanceZ:
		return FMath::Max(0.f, FMath::Abs(A.Z - B.Z) - HalfHeightSum);
	default:
		checkNoEntry();
	}
	return MAX_flt;
}

FString UBTService_Distance::GetGeometricDistanceDescription(FAIDistanceType GeometricDistanceType)
{
	static FString Desc[] = { TEXT("distance"), TEXT("2D distance"), TEXT("Z distance") };
	return ((int)GeometricDistanceType < UE_ARRAY_COUNT(Desc)) ? Desc[(int)GeometricDistanceType] : FString();
}

#if WITH_EDITOR

FName UBTService_Distance::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Decorator.ReachedMoveGoal.Icon");
}

#endif	// WITH_EDITOR
