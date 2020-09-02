// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Kismet/KismetInputLibraryExtensions.h"

UKismetInputLibraryEx::UKismetInputLibraryEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FName UKismetInputLibraryEx::Key_GetName(const FKey& Key)
{
	return Key.GetFName();
}
