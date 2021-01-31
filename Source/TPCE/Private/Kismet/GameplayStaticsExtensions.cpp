// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Kismet/GameplayStaticsExtensions.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/StaticMeshSocket.h"

UGameplayStaticsEx::UGameplayStaticsEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplayStaticsEx::BreakHitResultEx(const FHitResult& Hit, FName& MyBoneName, float& PenetrationDepth)
{
	MyBoneName = Hit.MyBoneName;
	PenetrationDepth = Hit.PenetrationDepth;
}

FString UGameplayStaticsEx::GetSocketTag(class USceneComponent* Target, FName InSocketName)
{
	if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Target))
	{
		if (UStaticMeshSocket const* Socket = StaticMeshComponent->GetSocketByName(InSocketName))
		{
			return Socket->Tag;
		}
	}

	return FString();
}

void UGameplayStaticsEx::SetSourceCubemapAngle(class USkyLightComponent* SkyLightComponent, float NewSourceCubemapAngle)
{
	SkyLightComponent->SourceCubemapAngle = NewSourceCubemapAngle;
	SkyLightComponent->SetCaptureIsDirty();
}
