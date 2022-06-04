// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Application/DetectInputProcessor.h"

bool FDetectInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	if (InAnalogInputEvent.GetKey().IsGamepadKey() && CurrentInputDevice != EPlayerControllerInputDevices::Gamepad && InAnalogInputEvent.GetAnalogValue() > 0.15f)
	{
		CurrentInputDevice = EPlayerControllerInputDevices::Gamepad;
		OnInputDeviceChanged.ExecuteIfBound(CurrentInputDevice);
	}

	return false;
}

bool FDetectInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey().IsGamepadKey() && CurrentInputDevice != EPlayerControllerInputDevices::Gamepad)
	{
		CurrentInputDevice = EPlayerControllerInputDevices::Gamepad;
		OnInputDeviceChanged.ExecuteIfBound(CurrentInputDevice);
	}
	else if (!InKeyEvent.GetKey().IsGamepadKey() && CurrentInputDevice != EPlayerControllerInputDevices::Keyboard)
	{
		CurrentInputDevice = EPlayerControllerInputDevices::Keyboard;
		OnInputDeviceChanged.ExecuteIfBound(CurrentInputDevice);
	}

	return false;
}

bool FDetectInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (CurrentInputDevice != EPlayerControllerInputDevices::Mouse && MouseEvent.GetCursorDelta().Size() > 2.f)
	{
		CurrentInputDevice = EPlayerControllerInputDevices::Mouse;
		OnInputDeviceChanged.ExecuteIfBound(CurrentInputDevice);
	}

	return false;
}

bool FDetectInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent)
{
	if (CurrentInputDevice != EPlayerControllerInputDevices::Mouse)
	{
		CurrentInputDevice = EPlayerControllerInputDevices::Mouse;
		OnInputDeviceChanged.ExecuteIfBound(CurrentInputDevice);
	}

	return false;
}

bool FDetectInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (CurrentInputDevice != EPlayerControllerInputDevices::Mouse)
	{
		CurrentInputDevice = EPlayerControllerInputDevices::Mouse;
		OnInputDeviceChanged.ExecuteIfBound(CurrentInputDevice);
	}

	return false;
}
