// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BTTaskNode.h"

#include "BTTask_PlayMontage.generated.h"

/**
 * Play indicated montage on Pawn controlled by BT.
 */
UCLASS()
class TPCE_API UBTTask_PlayMontage : public UBTTaskNode
{
	GENERATED_UCLASS_BODY()

	/** Montage asset to play. Note that it needs to match the skeleton of the pawn this BT is controlling. */
	UPROPERTY(EditAnywhere, Category="Node")
	UAnimMontage* MontageToPlay;

	/** Montage play rate. */
	UPROPERTY(EditAnywhere, Category="Node")
	float PlayRate;

	/** Montage starting position. */
	UPROPERTY(EditAnywhere, Category="Node")
	float StartingPosition;

	/** Montage starting section. */
	UPROPERTY(EditAnywhere, Category="Node")
	FName StartingSection;

	/** If specified, finish when a notify with this name is executed. */
	UPROPERTY(EditAnywhere, Category="Node")
	FName FinishOnNotify;

	/** Finish on the montage blending out instead of the montage ending. */
	UPROPERTY(EditAnywhere, Category="Node")
	bool bFinishOnBlendOut;

	/** Blend out time when task is aborted and the montage is stopped. */
	UPROPERTY(EditAnywhere, Category="Node")
	float AbortBlendOutTime;

	UPROPERTY()
	UBehaviorTreeComponent* MyOwnerComp;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR

protected:

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

private:

	bool IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload) const;
	void UnbindDelegates();

	TWeakObjectPtr<UAnimInstance> AnimInstancePtr;
	int32 MontageInstanceID;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
};
