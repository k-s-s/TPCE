// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "BehaviorTree/BTTask_TurnTo.h"

#include "AIController.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_TurnTo::UBTTask_TurnTo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Turn To";
	bNotifyTick = true;

	Speed = 1.f;
	SpeedCurveName = NAME_None;
	Tolerance = 5.f;
	bConstantSpeed = false;
	bYawOnly = true;

	// Accept only actors and vectors
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_TurnTo, BlackboardKey), AActor::StaticClass());
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_TurnTo, BlackboardKey));
}


EBTNodeResult::Type UBTTask_TurnTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UBTTask_TurnTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();

	if (MyController && MyController->GetPawn())
	{
		APawn* MyPawn = MyController->GetPawn();

		/*if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID());
			if (const AActor* TargetActor = Cast<AActor>(KeyValue))
			{
				TargetRotation = GetLookAtRotation(MyPawn->GetActorLocation(), TargetActor->GetActorLocation());
			}
		}
		else if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			const FVector TargetLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());
			if (FAISystem::IsValidLocation(TargetLocation))
			{
				TargetRotation = GetLookAtRotation(MyPawn->GetActorLocation(), TargetLocation);
			}
		}*/

		const FQuat InitialRotation = MyPawn->GetActorQuat();
		FVector TargetLocation = FVector::ZeroVector;
		FQuat TargetRotation = InitialRotation;
		if (MyBlackboard && MyBlackboard->GetLocationFromEntry(BlackboardKey.GetSelectedKeyID(), TargetLocation))
		{
			TargetRotation = GetLookAtRotation(MyPawn->GetActorLocation(), TargetLocation);
		}

		MyBlackboard->GetLocationFromEntry(BlackboardKey.GetSelectedKeyID(), TargetLocation);

		// Scale speed by animation value if the curve name is given
		float CurrentSpeed = Speed;
		if (SpeedCurveName != NAME_None)
		{
			if (const ACharacter* MyCharacter = Cast<ACharacter>(MyController->GetPawn()))
			{
				if (const USkeletalMeshComponent* SkelMesh = MyCharacter->GetMesh())
				{
					if (const UAnimInstance* AnimInstance = SkelMesh->GetAnimInstance())
					{
						CurrentSpeed *= AnimInstance->GetCurveValue(SpeedCurveName);
					}
				}
			}
		}

		FQuat NewRotation = InitialRotation;
		if (CurrentSpeed > 0.f)
		{
			if (bConstantSpeed)
			{
				NewRotation = FMath::QInterpConstantTo(InitialRotation, TargetRotation, DeltaSeconds, FMath::DegreesToRadians(CurrentSpeed));
			}
			else
			{
				NewRotation = FMath::QInterpTo(InitialRotation, TargetRotation, DeltaSeconds, CurrentSpeed);
			}
			MyPawn->SetActorRotation(NewRotation, ETeleportType::None);
		}

		// If tolerance is set and reached target, finish task
		if (Tolerance > 0.f)
		{
			const float DeltaDegrees = FMath::RadiansToDegrees(NewRotation.AngularDistance(TargetRotation));
			if (DeltaDegrees <= Tolerance)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}
	}
	else
	{
		// Invalid state
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

FQuat UBTTask_TurnTo::GetLookAtRotation(const FVector& StartLocation, const FVector& TargetLocation) const
{
	FVector DeltaLocation = TargetLocation - StartLocation;
	DeltaLocation.Z = 0.f;
	if (bYawOnly)
	{
		DeltaLocation.Z = 0.f;
	}
	return FRotationMatrix::MakeFromX(DeltaLocation).ToQuat();
}

FString UBTTask_TurnTo::GetStaticDescription() const
{
	FString KeyDesc("invalid");
	if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() ||
		BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		KeyDesc = BlackboardKey.SelectedKeyName.ToString();
	}

	return FString::Printf(TEXT("%s: %s"), *Super::GetStaticDescription(), *KeyDesc);
}

#if WITH_EDITOR

FName UBTTask_TurnTo::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.MoveTo.Icon");
}

#endif	// WITH_EDITOR
