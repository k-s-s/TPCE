// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ExtPlayerController.h"

#include "TopDownPlayerController.generated.h"

class UTopDownPushToTargetComponent;
class USphereComponent;
class UArmComponent;
class UCameraComponent;

/**
 * A player controller implementation for top-down games intended to possess a TopDownCharacter.
 *
 * @see TopDownCharacter
 */
UCLASS(Abstract, ShowCategories=(Collision))
class TPCE_API ATopDownPlayerController : public AExtPlayerController
{
	GENERATED_BODY()

public:

	ATopDownPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:

	/** Mounting point of the camera boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UTopDownPushToTargetComponent* CameraTractor;

	/** Camera mount used to support the camera boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CameraMount;

	/** Camera boom to position the camera at a fixed distance above the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UArmComponent* CameraBoom;

	/** Camera settings. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	/** Returns the corners of the view frustum intersection with the ground plane. */
	UFUNCTION(BlueprintCallable, Category = Camera)
	bool GetViewExtents(FVector& TopLeft, FVector& TopRight, FVector& BottomRight, FVector& BottomLeft, FVector& Min, FVector& Max) const;

	/** Draw debug helpers. */
	UPROPERTY(EditDefaultsOnly, Category = Camera, AdvancedDisplay)
	bool bDebugCamera;

protected:

	/** Force scene component to use absolute coordinates */
	void ForceAbsolute(USceneComponent* SceneComponent);

	void UpdateViewExtents();

	FVector ViewCorners[4];
	FVector ViewExtentsMin;
	FVector ViewExtentsMax;
	bool bViewExtentsValid;
	FMatrix OverlayTransform;

public:

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
	virtual bool HasActiveCameraComponent() const override;
	virtual bool HasActivePawnControlCameraComponent() const override;
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Camera")
	void SetCameraTargetComponent(USceneComponent* NewTargetComponent, const FName& SocketName = NAME_None);

	/** Camera mount used to support the camera boom and be moved by the camera trator. */
	FORCEINLINE USphereComponent* GetCameraMount() { return CameraMount; }

	/** Camera boom to position the camera at a fixed distance above the character. */
	FORCEINLINE UArmComponent* GetCameraBoom() { return CameraBoom; }

	/** Camera settings. */
	FORCEINLINE UCameraComponent* GetCamera() { return CameraComponent; }

	/** Camera tractor responsible for moving the camera mount towards the view target. */
	FORCEINLINE UTopDownPushToTargetComponent* GetCameraTractor() { return CameraTractor; }

};
