// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/KismetArrayLibraryExtensions.h"

UKismetArrayLibraryEx::UKismetArrayLibraryEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UKismetArrayLibraryEx::Array_ShuffleWithStream(const TArray<int32>& TargetArray, const FRandomStream& RandomStream)
{
	// We should never hit these!  They're stubs to avoid NoExport on the class.  Call the Generic* equivalent instead
	check(0);
}

void UKismetArrayLibraryEx::GenericArray_ShuffleWithStream(void* TargetArray, const UArrayProperty* ArrayProp, const FRandomStream& RandomStream)
{
	if (TargetArray)
	{
		FScriptArrayHelper ArrayHelper(ArrayProp, TargetArray);
		int32 LastIndex = ArrayHelper.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			int32 Index = RandomStream.RandRange(i, LastIndex);
			if (i != Index)
			{
				ArrayHelper.SwapValues(i, Index);
			}
		}
	}
}
