// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "MaterialParameterCollectionAssetContainer.generated.h"

/**
 * Simple data asset holding a reference to a MaterialParameterCollection.
 */
UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class TPCE_API UMaterialParameterCollectionAssetContainer : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMaterialParameterCollection* MaterialParameterCollection;
};
