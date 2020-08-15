// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/KismetInputLibrary.h"

#include "KismetInputLibraryExtensions.generated.h"


UCLASS(meta=(BlueprintThreadSafe))
class TPCE_API UKismetInputLibraryEx : public UKismetInputLibrary
{
	GENERATED_UCLASS_BODY()

public:

	/**
	 * Returns the name of the key.
	 */
	UFUNCTION(BlueprintPure, meta=(DisplayName="Get Key Name"), Category="Utilities|Key")
	static FName Key_GetName(const FKey& Key);
};
