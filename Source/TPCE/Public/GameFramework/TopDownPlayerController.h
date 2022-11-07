// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "GameFramework/ExtPlayerController.h"
#include "Engine/CanvasRenderTarget2D.h"

#include "TopDownPlayerController.generated.h"

class UTopDownPushToTargetComponent;
class USphereComponent;
class UArmComponent;
class UCameraComponent;
class UDecalComponent;
class UCanvasRenderTarget2D;

/**
 * A player controller implementation for top-down games intended to possess a TopDownCharacter.
 *
 * @see TopDownCharacter
 */
UCLASS(Abstract, ShowCategories=(Collision))
class TPCE_API ATopDownPlayerController : public AExtPlayerController
{
	GENERATED_BODY()

protected:

	/** Enables the ground overlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Overlay")
	bool bGroundOverlay;

	/** Height of the ground overlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Overlay")
	float OverlayZ;

	/** Depth of the decal used to draw the ground overlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Overlay")
	float OverlayHeight;

	/** Size of the ground overlay canvas. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Overlay", meta=(ClampMin="0"))
	int OverlayCanvasSize;

	/** Resolution of the ground overlay canvas. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Overlay", meta=(ClampMin="0"))
	float OverlayPixelsPerUnit;

	/**
	 * Allows Blueprint to implement how the ground overlay should be updated.
	 *
	 * @param	Canvas				Canvas object that can be used to paint to the render target
	 * @param	Width				Width of the render target.
	 * @param	Height				Height of the render target.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Ground Overlay", meta=(DisplayName="On Overlay Update"))
	void ReceiveOverlayUpdate(UCanvas* InCanvas, int32 Width, int32 Height);

	/** Called when the ground overlay is asked to update its texture resource. */
	UPROPERTY(BlueprintAssignable, Category="Ground Overlay")
	FOnCanvasRenderTargetUpdate OnOverlayUpdate;

public:

	ATopDownPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
	virtual bool HasActiveCameraComponent() const override;
	virtual bool HasActivePawnControlCameraComponent() const override;
	virtual FPlane GetGroundPlane() const;

	UFUNCTION(BlueprintCallable, Category="Camera")
	void SetCameraTargetComponent(USceneComponent* NewTargetComponent, const FName& SocketName=NAME_None);

protected:

	/** Force scene component to use absolute coordinates */
	void ForceAbsolute(USceneComponent* SceneComponent);

	UPROPERTY(Transient)
	UCanvasRenderTarget2D* OverlayCanvasRenderTarget;

	void UpdateGroundOverlay();
	FMatrix OverlayCanvasTransform;

	UFUNCTION()
	void DrawGroundOverlay(UCanvas* InCanvas, int32 Width, int32 Height);

private:

	/** Mounting point of the camera boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	UTopDownPushToTargetComponent* CameraTractor;

	/** Camera mount used to support the camera boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	USphereComponent* CameraMount;

	/** Camera boom to position the camera at a fixed distance above the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	UArmComponent* CameraBoom;

	/** Camera settings. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	UCameraComponent* CameraComponent;

	/** Decal component. */
	UPROPERTY()
	UDecalComponent* GroundOverlay;

public:

	/** Camera mount used to support the camera boom and be moved by the camera trator. */
	FORCEINLINE USphereComponent* GetCameraMount() { return CameraMount; }

	/** Camera boom to position the camera at a fixed distance above the character. */
	FORCEINLINE UArmComponent* GetCameraBoom() { return CameraBoom; }

	/** Camera settings. */
	FORCEINLINE UCameraComponent* GetCamera() { return CameraComponent; }

	/** Camera tractor responsible for moving the camera mount towards the view target. */
	FORCEINLINE UTopDownPushToTargetComponent* GetCameraTractor() { return CameraTractor; }
};
