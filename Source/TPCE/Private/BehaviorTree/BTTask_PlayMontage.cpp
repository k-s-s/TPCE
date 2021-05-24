// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "BehaviorTree/BTTask_PlayMontage.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "VisualLogger/VisualLogger.h"

UBTTask_PlayMontage::UBTTask_PlayMontage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Play Montage";
	bCreateNodeInstance = true;

	PlayRate = 1.f;
	StartingPosition = 0.f;
	StartingSection = NAME_None;
	bFinishOnBlendOut = false;
	FinishOnNotify = NAME_None;

	MontageInstanceID = INDEX_NONE;
}

EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* MyController = OwnerComp.GetAIOwner();
	EBTNodeResult::Type Result = EBTNodeResult::Failed;
	MyOwnerComp = &OwnerComp;

	if (MontageToPlay && MyController && MyController->GetPawn())
	{
		USkeletalMeshComponent* SkelMesh = nullptr;
		if (const ACharacter* MyCharacter = Cast<ACharacter>(MyController->GetPawn()))
		{
			SkelMesh = MyCharacter->GetMesh();
		}
		else
		{
			SkelMesh = MyController->GetPawn()->FindComponentByClass<USkeletalMeshComponent>();
		}

		if (SkelMesh != nullptr)
		{
			if (UAnimInstance* AnimInstance = SkelMesh->GetAnimInstance())
			{
				const float MontageLength = AnimInstance->Montage_Play(MontageToPlay, PlayRate, EMontagePlayReturnType::MontageLength, StartingPosition);
				if (MontageLength > 0.f)
				{
					AnimInstancePtr = AnimInstance;
					if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay))
					{
						MontageInstanceID = MontageInstance->GetInstanceID();
					}

					if (StartingSection != NAME_None)
					{
						AnimInstance->Montage_JumpToSection(StartingSection, MontageToPlay);
					}

					BlendingOutDelegate.BindUObject(this, &UBTTask_PlayMontage::OnMontageBlendingOut);
					AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

					MontageEndedDelegate.BindUObject(this, &UBTTask_PlayMontage::OnMontageEnded);
					AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

					AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UBTTask_PlayMontage::OnNotifyBeginReceived);
					AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UBTTask_PlayMontage::OnNotifyEndReceived);

					Result = EBTNodeResult::InProgress;
				}
				else
				{
					UE_VLOG(MyController, LogBehaviorTree, Log, TEXT("%s> Instant success due to having a valid MontageToPlay and Character with SkelMesh, but montage failed to play"), *GetNodeName());
					// We're done here, report success so that BT can pick next task
					Result = EBTNodeResult::Succeeded;
				}
			}
		}
	}

	return Result;
}

EBTNodeResult::Type UBTTask_PlayMontage::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnbindDelegates();

	return EBTNodeResult::Aborted;
}

FString UBTTask_PlayMontage::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: '%s'"), *Super::GetStaticDescription(), *GetNameSafe(MontageToPlay));
}

bool UBTTask_PlayMontage::IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload) const
{
	return ((MontageInstanceID != INDEX_NONE) && (BranchingPointNotifyPayload.MontageInstanceID == MontageInstanceID));
}

void UBTTask_PlayMontage::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (MyOwnerComp && bFinishOnBlendOut)
	{
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
		UnbindDelegates();
	}
}

void UBTTask_PlayMontage::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (MyOwnerComp && !bFinishOnBlendOut)
	{
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
		UnbindDelegates();
	}
}

void UBTTask_PlayMontage::OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (MyOwnerComp && IsNotifyValid(NotifyName, BranchingPointNotifyPayload))
	{
		if (FinishOnNotify != NAME_None && NotifyName == FinishOnNotify)
		{
			FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
		}

		//OnNotifyBegin.Broadcast(NotifyName);
	}
}

void UBTTask_PlayMontage::OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (MyOwnerComp && IsNotifyValid(NotifyName, BranchingPointNotifyPayload))
	{
		//OnNotifyEnd.Broadcast(NotifyName);
	}
}

void UBTTask_PlayMontage::UnbindDelegates()
{
	if (UAnimInstance* AnimInstance = AnimInstancePtr.Get())
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UBTTask_PlayMontage::OnNotifyBeginReceived);
		AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &UBTTask_PlayMontage::OnNotifyEndReceived);
	}

	AnimInstancePtr.Reset();
	MyOwnerComp = nullptr;
}

#if WITH_EDITOR

FName UBTTask_PlayMontage::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.PlaySound.Icon");
}

#endif // WITH_EDITOR
