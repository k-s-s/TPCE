// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "Framework/Application/IInputProcessor.h"
#include "GameFramework/ExtPlayerController.h"

DECLARE_DELEGATE_OneParam(FInputDeviceChangedSignature, EPlayerControllerInputDevices);

/**
 * Reports whenever the latest input device changes to a different device.
 * To use you must create and register an instance of FDetectInputProcessor, possibly in your game module's StartupModule().
 * Note that LoadingPhase should be at least PreLoadingScreen, or FSlateApplication won't be initialized.
 *
 * if (FSlateApplication::IsInitialized())
 * {
 *		DetectInputProcessor = MakeShared<FDetectInputProcessor>();
 *		DetectInputProcessor->OnInputDeviceChanged.BindRaw(this, &FKemokoGameModule::InputDeviceChangedHandler);
 *		FSlateApplication::Get().RegisterInputPreProcessor(DetectInputProcessor);
 * }
 */
class TPCE_API FDetectInputProcessor : public IInputProcessor
{
public:

	// Begin IInputProcessor Interface
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {};
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override;
	virtual bool HandleMouseButtonDownEvent( FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	// End IInputProcessor Interface

	FInputDeviceChangedSignature OnInputDeviceChanged;

protected:

	EPlayerControllerInputDevices CurrentInputDevice;
};
