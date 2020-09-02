// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/KismetGuidLibrary.h"
#include "Kismet/KismetInputLibrary.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetNodeHelperLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetStringTableLibrary.h"
#include "Kismet/KismetTextLibrary.h"
#include "KismetArrayLibraryExtensions.h"
#include "KismetInputLibraryExtensions.h"
#include "KismetMathLibraryExtensions.h"

static struct TPCE_API Kismet: public UKismetSystemLibrary
{
	typedef ::UKismetArrayLibraryEx Array;
	typedef ::UKismetGuidLibrary Guid;
	typedef ::UKismetInputLibraryEx Input;
	typedef ::UKismetInternationalizationLibrary i18n;
	typedef ::UKismetMaterialLibrary Material;
	typedef ::UKismetMathLibraryEx Math;
	typedef ::UKismetNodeHelperLibrary Util;
	typedef ::UKismetRenderingLibrary Rendering;
	typedef ::UKismetStringLibrary String;
	typedef ::UKismetStringTableLibrary StringTable;
	typedef ::UKismetTextLibrary Text;
};


/**
 *
 */
template <typename EnumType>
FORCEINLINE FString GetEnumeratorUserFriendlyName(const TCHAR* EnumClass, const EnumType EnumValue)
{
	if (const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, EnumClass, true))
		return EnumPtr->GetDisplayNameTextByValue((uint8)EnumValue).ToString();

	return FName().ToString();
}

/**
 *
 */
#define GetEnumeratorDisplayName(EnumClass, EnumValue)  GetEnumeratorUserFriendlyName(TEXT(#EnumClass), (EnumValue))
