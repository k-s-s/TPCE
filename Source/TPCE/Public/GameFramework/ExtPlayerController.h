// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"

#include "ExtPlayerController.generated.h"

class UInputComponent;

/** A set of parameters to describe how to transition between view targets. */
USTRUCT(BlueprintType)
struct TPCE_API FAutoManagedCameraTransitionParams
{
	GENERATED_USTRUCT_BODY()

	FAutoManagedCameraTransitionParams()
		: BlendTime(0.f)
  		, BlendFunction(VTBlend_Cubic)
		, BlendExp(2.f)
		, bFadeToBlack(false)
		, bShouldFadeAudio(false)
	{}


	/** Total duration of blend to pending view target. 0 means no blending. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ViewTargetTransitionParams)
	float BlendTime;

	/** Function to apply to the blend parameter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ViewTargetTransitionParams)
	TEnumAsByte<enum EViewTargetBlendFunction> BlendFunction;

	/** Exponent, used by certain blend functions to control the shape of the curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ViewTargetTransitionParams)
	float BlendExp;

	/** Fade to black instead of interpolating the view. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ViewTargetTransitionParams)
	bool bFadeToBlack;

	/** Lower audio volume while fading to black. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ViewTargetTransitionParams)
	bool bShouldFadeAudio;

	operator FViewTargetTransitionParams() const
	{
		FViewTargetTransitionParams params;
		params.BlendTime = BlendTime;
		params.BlendFunction = BlendFunction;
		params.BlendExp = BlendExp;
		params.bLockOutgoing = true;

		return params;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInputStackChangedSignature);

/** Collected key mappings that share the same description. */
USTRUCT(BlueprintType)
struct TPCE_API FInputBindingDescription
{
	GENERATED_USTRUCT_BODY()

	/** Mapping between an action and key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInputActionKeyMapping> KeyMappings;

	/** User provided description. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;
};

/** Specifies sets of device types. */
UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EPlayerControllerInputDevices : uint8
{
	None = 0x00 UMETA(Hidden),
	Keyboard = 0x01,
	Mouse = 0x02,
	Gamepad = 0x04,
	Touch = 0x08,
};
ENUM_CLASS_FLAGS(EPlayerControllerInputDevices);

/**
 * Base class for player controllers with extended functionality.
 * This class exists basically to modify (or fix) the base APlayerController implementation
 * without having to patch the engine. It also supports IPawnControlInterface to provide
 * extra features for Pawns.
 *
 */
UCLASS(Abstract)
class TPCE_API AExtPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AExtPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:

	/** A set of parameters to describe how to transition between view targets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FAutoManagedCameraTransitionParams AutoManagedCameraTransitionParams;

	virtual void SetupInputComponent() override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void AutoManageActiveCameraTarget(AActor* SuggestedTarget) override;
	virtual void BeginPlay() override;
	virtual class AActor* GetViewTarget() const override;
	virtual void SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void PushInputComponent(UInputComponent* Input) override;
	virtual bool PopInputComponent(UInputComponent* Input) override;

	/** Collect key mappings for bindings that have an user provided description. Set InputDevices to filter the result. */
	UFUNCTION(BlueprintCallable, Category="Input")
	void GetInputBindingDescriptions(TArray<FInputBindingDescription>& OutInputBindingDescriptions);

	/** Occurs when the input stack is changed. */
	UPROPERTY(BlueprintAssignable, Category="Input")
	FInputStackChangedSignature OnInputStackChanged;

	/** Specified set of input devices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input", meta=(Bitmask, BitmaskEnum="EPlayerControllerInputDevices"))
	int32 InputDevices;

	/** Convert current mouse 2D position to World Space 3D position projected on to a plane. Returns false if unable to determine value. **/
	UFUNCTION(BlueprintCallable, Category = "Game|Player", meta = (DisplayName = "ProjectMouseLocationOnToPlane", Keywords = "deproject"))
	bool DeprojectMousePositionToPlane(FVector& WorldLocation, const FPlane& Plane) const;

	/** Convert current mouse 2D position to World Space 3D position projected on to the ground plane. Returns false if unable to determine value. **/
	UFUNCTION(BlueprintCallable, Category = "Game|Player", meta = (DisplayName = "ProjectMouseLocationOnToGround", Keywords = "deproject"))
	bool DeprojectMousePositionToGround(FVector& WorldLocation) const;

	/** Convert 2D screen position to World Space 3D position projected on to a plane. Returns false if unable to determine value. **/
	UFUNCTION(BlueprintCallable, Category = "Game|Player", meta = (DisplayName = "ProjectScreenLocationOnToPlane", Keywords = "deproject"))
	bool DeprojectScreenPositionToPlane(float ScreenX, float ScreenY, FVector& WorldLocation, const FPlane& Plane) const;

	/** Convert 2D screen position to World Space 3D position projected on to the ground plane. Returns false if unable to determine value. **/
	UFUNCTION(BlueprintCallable, Category = "Game|Player", meta = (DisplayName = "ProjectScreenLocationOnToGround", Keywords = "deproject"))
	bool DeprojectScreenPositionToGround(float ScreenX, float ScreenY, FVector& WorldLocation) const;

	/** Returns the corners of the view frustum intersection with the ground plane. */
	UFUNCTION(BlueprintCallable, Category = Camera)
	bool GetViewExtents(FVector& TopLeft, FVector& TopRight, FVector& BottomRight, FVector& BottomLeft, FVector& Min, FVector& Max) const;

	/** Returns the ground plane used for view extents and other functions. */
	virtual FPlane GetGroundPlane() const;

	/**
	 * Event when the view fades to black after SetViewTargetWithFade is called, before setting the new view target.
	 * Set PendingFadeViewTarget to None to prevent the transition from occurring.
	*/
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Fade To Black"))
	void ReceiveFadeToBlack(AActor* NewViewTarget);

	/**
	 * Set the view target with a camera fade to a solid color.
	 * @param NewViewTarget - New actor to set as view target.
	 * @param BlendTime - How long the fade should take, in seconds.
	 * @param Color - Color to fade to/from.
	 * @param bShouldFadeAudio - True to fade audio volume along with the alpha of the solid color.
	 */
	UFUNCTION(BlueprintCallable, Category="Game|Player", meta=(Keywords="Camera"))
	virtual void SetViewTargetWithFade(AActor* NewViewTarget, float BlendTime, FLinearColor Color, bool bShouldFadeAudio = false);

	/** Pending view target after camera fades. */
	UPROPERTY(Transient, BlueprintReadWrite)
	AActor* PendingFadeViewTarget;

	/** Draw debug helpers. */
	UPROPERTY(EditDefaultsOnly, Category = Camera, AdvancedDisplay)
	bool bDebugCamera;

protected:

	EPlayerControllerInputDevices GetKeyInputDevices(FKey Key);
	void UpdateViewExtents();

	UPROPERTY(Transient)
	AActor* OldViewTarget;

	UPROPERTY(Transient)
	bool bBeganPlaying;

	FVector ViewCorners[4];
	FVector ViewExtentsMin;
	FVector ViewExtentsMax;
	bool bViewExtentsValid;
	FMatrix OverlayTransform;
};
