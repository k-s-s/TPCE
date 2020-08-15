// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/KismetInputLibraryExtensions.h"

UKismetInputLibraryEx::UKismetInputLibraryEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FName UKismetInputLibraryEx::Key_GetName(const FKey& Key)
{
	return Key.GetFName();
}
