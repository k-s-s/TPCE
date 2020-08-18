// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/GameplayStaticsExtensions.h"
#include "Components/StaticMeshComponent.h"
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
