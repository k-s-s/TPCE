// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"

#include "ExtPlayerCameraManager.generated.h"

/**
 * Base class for player camera managers with extended functionality.
 */
UCLASS()
class TPCE_API AExtPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	/** Get the time remaining in viewtarget blend. */
	UFUNCTION(BlueprintCallable, Category=PlayerCameraManager)
	float GetBlendTimeToGo() const;

	/** Get the total progress in viewtarget blend. */
	UFUNCTION(BlueprintCallable, Category=PlayerCameraManager)
	float GetBlendTimePercent() const;

	/** Retrieve the focal point the camera should focus to on given actor. */
	UFUNCTION(BlueprintCallable, Category=PlayerCameraManager)
	FVector GetFocalPointOnActor(const AActor *Actor) const;
};
