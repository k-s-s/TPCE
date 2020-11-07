// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "Animation/AnimTypes.h"
#include "Misc/WildcardString.h"

#include "AnimationModifier_RemoveBones.generated.h"

class UAnimSequence;

/**
 * Animation Modifier to remove sets of bones.
 */
UCLASS(meta = (DisplayName="Remove Bones"))
class UAnimationModifier_RemoveBones : public UAnimationModifier
{
	GENERATED_BODY()

public:
	UAnimationModifier_RemoveBones();

public:
	/** Only animations with a path containing this filter as a case insensitive substring will be affected. Empty value matches all. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
	FName PathFilter;

	/**
	 * An array of filters specifying the bones that should be removed.
	 * Wildcards '?' and '*' are allowed. Prefix with '+' to include children.
	 *
	 * Examples: "+head", "hair_*", "twist_??_L"
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
	TArray<FString> BoneFilters;

	/**
	 * An array of filters specifying the curves that should be removed.
	 * Wildcards '?' and '*' are allowed.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
	TArray<FString> CurveFilters;

	/** If True, bone filters ignore case. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
	bool bCaseInsensitive;

	/** Invert the result. If True, the list specifies the bones that are KEPT. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
	bool bInvertSelection;

	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
};
