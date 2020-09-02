// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "AnimNotifies/AnimNotifyState_MatchTarget.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

UAnimNotifyState_MatchTarget::UAnimNotifyState_MatchTarget()
	: MatchTargetGetterName(TEXT("GetMatchTarget"))
	, bMatchPosition(true)
	, bMatchRotation(true)
	, bMatchScale(false)
	, bSweep(false)
	, Teleport(ETeleportType::None)
	, EaseFunction(EEasingFunc::EaseInOut)
	, BlendExponent(2.0f)
{
}

void UAnimNotifyState_MatchTarget::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	Elapsed = 0.0f;
	this->Duration = TotalDuration;
	Actor = MeshComp->GetOwner();
	OriginTM = Actor->GetActorTransform();
	DestinationTM = OriginTM;

	if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
	{
		if (UFunction* MatchTargetGetter = AnimInstance->FindFunction(MatchTargetGetterName))
		{
			struct FGetMatchTargetParams
			{
				FName NotifyName;
				FTransform RetVal;
			} Params;

			Params.NotifyName = FName(*GetNotifyName());
			AnimInstance->ProcessEvent(MatchTargetGetter, &Params);
			DestinationTM = Params.RetVal;
		}
	}
}

void UAnimNotifyState_MatchTarget::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	Elapsed += FrameDeltaTime;
	float Alpha = FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f);

	UpdateActorTransform(Alpha);
}

void UAnimNotifyState_MatchTarget::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	UpdateActorTransform(1.0f);
}

void UAnimNotifyState_MatchTarget::UpdateActorTransform(float Alpha)
{
	if (!Actor.IsValid())
	{
		return;
	}

	const FTransform& CurrentTransform = Actor->GetActorTransform();
	FTransform Target = DestinationTM;

	if (ACharacter* Character = Cast<ACharacter>(Actor))
	{
		FVector CapsuleOffset = CurrentTransform.GetRotation().GetUpVector() * Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		Target.AddToTranslation(CapsuleOffset);
	}

	FTransform NewTransform = BlendTransform(OriginTM, Target, Alpha);

	if (!bMatchPosition)
	{
		NewTransform.CopyTranslation(CurrentTransform);
	}

	if (!bMatchRotation)
	{
		NewTransform.CopyRotation(CurrentTransform);
	}

	if (!bMatchScale)
	{
		NewTransform.CopyScale3D(CurrentTransform);
	}

	FHitResult OutSweepHitResult;
	Actor->SetActorTransform(NewTransform, bSweep, &OutSweepHitResult, Teleport);
}

FTransform UAnimNotifyState_MatchTarget::BlendTransform(const FTransform& Origin, const FTransform& Destination, float Alpha) const
{
	return UKismetMathLibrary::TEase(Origin, Destination, Alpha, EaseFunction, BlendExponent);
}

#if WITH_EDITOR

bool UAnimNotifyState_MatchTarget::CanBePlaced(UAnimSequenceBase* Animation) const
{
	return Animation && Animation->IsA(UAnimMontage::StaticClass());
}

#endif // WITH_EDITOR
