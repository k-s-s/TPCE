// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "BehaviorTree/BTDecorator_DistanceCheck.h"

#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "AIController.h"

UBTDecorator_DistanceCheck::UBTDecorator_DistanceCheck(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Distance Check";
	MinDistance = 0.f;
	MaxDistance = 0.f;
	GeometricDistanceType = FAIDistanceType::Distance3D;

	// Accept only actors and vectors
	Observed.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_DistanceCheck, Observed), AActor::StaticClass());
	Observed.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_DistanceCheck, Observed));

	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
	FlowAbortMode = EBTFlowAbortMode::None;
}

void UBTDecorator_DistanceCheck::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	MinDistanceSqr = FMath::Square(MinDistance);
	MaxDistanceSqr = FMath::Square(MaxDistance);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		Observed.ResolveSelectedKey(*BBAsset);
	}
}

bool UBTDecorator_DistanceCheck::CalculateDistance(const UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& Target, float& DistanceSqr) const
{
	FVector TargetLocation = FVector::ZeroVector;
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	const AAIController* MyController = OwnerComp.GetAIOwner();

	if (MyBlackboard && MyBlackboard->GetLocationFromEntry(Target.GetSelectedKeyID(), TargetLocation) && MyController && MyController->GetPawn())
	{
		DistanceSqr = GetGeometricDistanceSquared(MyController->GetPawn()->GetActorLocation(), TargetLocation);
		return true;
	}

	return false;
}


FORCEINLINE bool UBTDecorator_DistanceCheck::CalcConditionImpl(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	float DistanceSqr;
	return CalculateDistance(OwnerComp, Observed, DistanceSqr)
		&& (MinDistanceSqr <= 0.f || DistanceSqr >= MinDistanceSqr) && (MaxDistanceSqr <= 0.f || DistanceSqr <= MaxDistanceSqr);
}

bool UBTDecorator_DistanceCheck::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return CalcConditionImpl(OwnerComp, NodeMemory);
}

void UBTDecorator_DistanceCheck::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TNodeInstanceMemory* MyMemory = (TNodeInstanceMemory*)NodeMemory;
	MyMemory->bLastRawResult = CalcConditionImpl(OwnerComp, NodeMemory);
}

void UBTDecorator_DistanceCheck::OnBlackboardChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	check(false);
}

void UBTDecorator_DistanceCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	TNodeInstanceMemory* MyMemory = CastInstanceNodeMemory<TNodeInstanceMemory>(NodeMemory);

	const bool bResult = CalcConditionImpl(OwnerComp, NodeMemory);
	if (bResult != MyMemory->bLastRawResult)
	{
		MyMemory->bLastRawResult = bResult;
		OwnerComp.RequestExecution(this);
	}
}

float UBTDecorator_DistanceCheck::GetGeometricDistanceSquared(const FVector& A, const FVector& B) const
{
	float Result = MAX_flt;
	switch (GeometricDistanceType)
	{
	case FAIDistanceType::Distance3D:
		Result = FVector::DistSquared(A, B);
		break;
	case FAIDistanceType::Distance2D:
		Result = FVector::DistSquaredXY(A, B);
		break;
	case FAIDistanceType::DistanceZ:
		Result = FMath::Square(A.Z - B.Z);
		break;
	default:
		checkNoEntry();
		break;
	}
	return Result;
}

FString UBTDecorator_DistanceCheck::GetStaticDescription() const
{
	FString KeyDesc("invalid");
	if (Observed.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() ||
		Observed.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		KeyDesc = Observed.SelectedKeyName.ToString();
	}

	if (MinDistance > 0.f && MaxDistance > 0.f)
	{
		return FString::Printf(TEXT("%s: is %s between %.2f and %.2f units away"), *Super::GetStaticDescription(), *KeyDesc, MinDistance, MaxDistance);
	}
	else if (MinDistance > 0.f)
	{
		return FString::Printf(TEXT("%s: is %s farther than %.2f units"), *Super::GetStaticDescription(), *KeyDesc, MinDistance);
	}
	else if (MaxDistance > 0.f)
	{
		return FString::Printf(TEXT("%s: is %s closer than %.2f units"), *Super::GetStaticDescription(), *KeyDesc, MaxDistance);
	}

	return FString::Printf(TEXT("%s: no min/max distance set"), *Super::GetStaticDescription());
}

void UBTDecorator_DistanceCheck::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	const UBlackboardComponent* BBComponent = OwnerComp.GetBlackboardComponent();

	float Distance;
	if (CalculateDistance(OwnerComp, Observed, Distance))
	{
		Values.Add(FString::Printf(TEXT("Distance: %.2f (%s range)"),
			Distance,
			(MinDistance <= 0.f || Distance >= MinDistance) && (MaxDistance <= 0.f || Distance <= MaxDistance) ? TEXT("inside") : TEXT("outside")
		));
	}
}

#if WITH_EDITOR

FName UBTDecorator_DistanceCheck::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Decorator.ReachedMoveGoal.Icon");
}

#endif	// WITH_EDITOR
