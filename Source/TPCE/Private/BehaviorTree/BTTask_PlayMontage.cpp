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
	FinishOnNotify = NAME_None;
	bFinishOnBlendOut = false;
	AbortBlendOutTime = .25f;

	MontageInstanceID = INDEX_NONE;
}

EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	MyOwnerComp = &OwnerComp;

	const AAIController* MyController = OwnerComp.GetAIOwner();
	EBTNodeResult::Type Result = EBTNodeResult::Failed;

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
				if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay))
				{
					AnimInstancePtr = AnimInstance;
					MontageInstanceID = MontageInstance->GetInstanceID();

					BlendingOutDelegate.BindUObject(this, &UBTTask_PlayMontage::OnMontageBlendingOut);
					MontageInstance->OnMontageBlendingOutStarted = BlendingOutDelegate;

					MontageEndedDelegate.BindUObject(this, &UBTTask_PlayMontage::OnMontageEnded);
					MontageInstance->OnMontageEnded = MontageEndedDelegate;

					AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UBTTask_PlayMontage::OnNotifyBeginReceived);

					if (StartingSection != NAME_None)
					{
						AnimInstance->Montage_JumpToSection(StartingSection, MontageToPlay);
					}

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

	if (UAnimInstance* AnimInstance = AnimInstancePtr.Get())
	{
		AnimInstance->Montage_Stop(AbortBlendOutTime, MontageToPlay);
	}

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
	if (MyOwnerComp && !bInterrupted && bFinishOnBlendOut)
	{
		UnbindDelegates();
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_PlayMontage::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (MyOwnerComp && !bInterrupted && !bFinishOnBlendOut)
	{
		UnbindDelegates();
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
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
	}
}

void UBTTask_PlayMontage::UnbindDelegates()
{
	MontageEndedDelegate.Unbind();
	BlendingOutDelegate.Unbind();

	if (UAnimInstance* AnimInstance = AnimInstancePtr.Get())
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UBTTask_PlayMontage::OnNotifyBeginReceived);
	}
}

#if WITH_EDITOR

FName UBTTask_PlayMontage::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.PlaySound.Icon");
}

#endif // WITH_EDITOR
