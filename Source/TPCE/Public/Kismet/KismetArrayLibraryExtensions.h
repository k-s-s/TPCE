// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "Kismet/KismetArrayLibrary.h"

#include "KismetArrayLibraryExtensions.generated.h"


UCLASS(meta=(BlueprintThreadSafe))
class TPCE_API UKismetArrayLibraryEx : public UKismetArrayLibrary
{
	GENERATED_UCLASS_BODY()

public:

	/**
	 * Shuffle (randomize) the elements of an array
	 *
	 * @param	TargetArray		The array to shuffle
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName="Shuffle with Stream", CompactNodeTitle="SHUFFLE", ArrayParm="TargetArray"), Category="Utilities|Array")
	static void Array_ShuffleWithStream(const TArray<int32>& TargetArray, const FRandomStream& RandomStream);

	// Native functions that will be called by the below custom thunk layers, which read off the property address, and call the appropriate native handler
	static void GenericArray_ShuffleWithStream(void* TargetArray, const UArrayProperty* ArrayProp, const FRandomStream& RandomStream);

	DECLARE_FUNCTION(execArray_ShuffleWithStream)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<UArrayProperty>(NULL);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;
		UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_GET_STRUCT_REF(FRandomStream, RandomStream);

		P_FINISH;
		P_NATIVE_BEGIN;
		GenericArray_ShuffleWithStream(ArrayAddr, ArrayProperty, RandomStream);
		P_NATIVE_END;
	}
};
