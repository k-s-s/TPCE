// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"

#include "ExtCameraModifier.generated.h"

/**
 * Base class for camera modifiers with extended functionality.
 *
 * Allows subclasses and blueprint to modify the view rotation.
 * Provides a way to temporarily pause the camera modifier, manually or automatically when the player looks around.
 */
UCLASS(BlueprintType, Blueprintable)
class TPCE_API UExtCameraModifier : public UCameraModifier
{
	GENERATED_BODY()

public:
	UExtCameraModifier();

	// Begin UCameraModifier Interface
	virtual void AddedToCamera(class APlayerCameraManager* Camera) override;
	virtual bool ModifyCamera(float DeltaTime, struct FMinimalViewInfo& InOutPOV) override;
	virtual bool ProcessViewRotation(class AActor* ViewTarget, float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot) override;
	virtual float GetTargetAlpha() override;
	virtual void UpdateAlpha(float DeltaTime);
	// End UCameraModifier Interface

	/**
	 * Called per tick that the modifier is active to allow modifiers to modify the camera's transform.
	 *
	 * @param	DeltaTime			Frame time in seconds.
	 * @param	ViewLocation		In/out. The camera location to modify.
	 * @param	ViewRotation		In/out. The camera rotation to modify.
	 * @param	FOV					In/out. The camera FOV to modify.
	 * @return	Return true to prevent subsequent (lower priority) modifiers to further adjust rotation, false otherwise.
	 */
	virtual bool NativeModifyCamera(float DeltaTime, FVector& ViewLocation, FRotator& ViewRotation, float& FOV);

	/**
	 * Called to give modifiers a chance to adjust view rotation updates before they are applied.
	 * Default just returns ViewRotation unchanged.
	 *
	 * @param	ViewTarget			Current view target.
	 * @param	DeltaTime			Frame time in seconds.
	 * @param	DeltaRot			The current input rotation delta.
	 * @param	OutViewRotation		In/out. The view rotation to modify.
	 * @return	Return true to prevent subsequent (lower priority) modifiers to further adjust rotation, false otherwise.
	 */
	virtual bool NativeProcessViewRotation(class AActor* ViewTarget, float DeltaTime, const FRotator& DeltaRot, FRotator& OutViewRotation);

	/**
	 * Called to give modifiers a chance to adjust view rotation updates before they are applied.
	 * Scaling by Alpha happens after this in code, so no need to deal with that in the blueprint.
	 *
	 * @param	ViewTarget			Current view target.
	 * @param	DeltaTime			Change in time since last update.
	 * @param	DeltaRot			The current input rotation delta.
	 * @param	ViewRotation		The current view rotation.
	 * @param	NewViewRotation		(out) The modified view rotation.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic)
	void BlueprintProcessViewRotation(class AActor* ViewTarget, float DeltaTime, FRotator DeltaRot, FRotator ViewRotation, FRotator& NewViewRotation);

	/**
	 * Allows any custom initialization. Called immediately after creation.
	 * @param Camera - The camera this modifier is associated with.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic)
	void BlueprintAddedToCamera(APlayerCameraManager* Camera);

	/** @return Returns true if modifier is paused, false otherwise. */
	UFUNCTION(BlueprintCallable, Category=CameraModifier)
	virtual bool IsPaused() const;

	/** If true, the modifier is paused (not disabled) on player rotation input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraModifier)
	bool bPauseWithPlayerInput;

	/** Unpauses the modifier after this many seconds pass with no player input. If zero, automatic unpause is disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraModifier, meta=(ClampMin=0))
	float UnpauseAfter;

	/** Smoothed alpha curve exponent. */
	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category=CameraModifier, meta=(ClampMin=0))
	float AlphaSmoothingExp;

	/** @return Returns the pawn associated with the owner player controller. */
	class APawn* GetPawn() const;

protected:
	float TimePaused;

	/** If true, do not apply this modifier to the camera. */
	uint32 bPaused:1;

	/** Current smoothed blend alpha. */
	UPROPERTY(Transient, BlueprintReadOnly, Category=CameraModifier)
	float SmoothAlpha;
};
