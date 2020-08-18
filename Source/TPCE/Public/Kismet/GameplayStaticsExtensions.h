// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/GameplayStatics.h"

#include "GameplayStaticsExtensions.generated.h"


UCLASS()
class TPCE_API UGameplayStaticsEx : public UGameplayStatics
{
	GENERATED_UCLASS_BODY()

public:

	/** Extracts additional data from a HitResult. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Collision")
	static void BreakHitResultEx(const struct FHitResult& Hit, FName& MyBoneName, float& PenetrationDepth);

	/**
	* Return the tag associated with the socket if it exists.
	* @param InSocketName Name of the socket to get the tag from.
	* @return The socket's tag if the socket with the given name exists. Otherwise, an empty string.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Utilities|Transformation", meta=(Keywords="Bone"))
	static FString GetSocketTag(class USceneComponent* Target, FName InSocketName);
};
