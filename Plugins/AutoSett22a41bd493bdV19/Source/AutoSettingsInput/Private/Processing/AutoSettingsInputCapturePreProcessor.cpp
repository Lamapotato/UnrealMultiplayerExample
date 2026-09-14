// Copyright Sam Bonifacio. All Rights Reserved.

#include "Processing/AutoSettingsInputCapturePreProcessor.h"

#include "Engine/LocalPlayer.h"
#include "AutoSettingsInputLogs.h"

bool FAutoSettingsInputCapturePreProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (ShouldIgnoreEvent(InKeyEvent))
	{
		return true;
	}

	KeysDown.AddUnique(InKeyEvent.GetKey());

	if (CaptureMode == EBindingCaptureMode::OnPressed)
	{
		Capture(InKeyEvent.GetKey());
		KeysDown.Remove(InKeyEvent.GetKey());
	}

	return true;
}

bool FAutoSettingsInputCapturePreProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (ShouldIgnoreEvent(InKeyEvent))
	{
		return true;
	}

	if (CaptureMode != EBindingCaptureMode::OnReleased)
	{
		KeysDown.Remove(InKeyEvent.GetKey());
		return true;
	}

	// Require that this key was pressed down while we were listening,
	// otherwise you can eg. press down a key, which opens the bind prompt,
	// then release the key but have the bind prompt capture it as a binding, closing the prompt immediately
	if (!KeysDown.Contains(InKeyEvent.GetKey()))
	{
		return true;
	}

	// Fire capture event
	Capture(InKeyEvent.GetKey());

	KeysDown.Remove(InKeyEvent.GetKey());
	return true;
}

bool FAutoSettingsInputCapturePreProcessor::HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp,
	const FPointerEvent& MouseEvent)
{
	return true;
}

bool FAutoSettingsInputCapturePreProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp,
	const FPointerEvent& MouseEvent)
{
	if (ShouldIgnoreEvent(MouseEvent))
	{
		return true;
	}

	UE_LOG(LogAutoSettingsInput, Verbose, TEXT("BindCapturePrompt: NativeOnMouseButtonDown"));

	KeysDown.AddUnique(MouseEvent.GetEffectingButton());

	if (CaptureMode == EBindingCaptureMode::OnPressed)
	{
		// Fire capture event
		Capture(MouseEvent.GetEffectingButton());

		KeysDown.Remove(MouseEvent.GetEffectingButton());
	}

	return true;
}

bool FAutoSettingsInputCapturePreProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp,
	const FPointerEvent& MouseEvent)
{
	if (!ShouldIgnoreEvent(MouseEvent))
	{
		UE_LOG(LogAutoSettingsInput, Verbose, TEXT("BindCapturePrompt: NativeOnMouseButtonUp"));
		if (CaptureMode != EBindingCaptureMode::OnReleased)
		{
			return true;
		}

		// Fire capture event
		Capture(MouseEvent.GetEffectingButton());

		KeysDown.Remove(MouseEvent.GetEffectingButton());

		return true;
	}

	return true;
}

bool FAutoSettingsInputCapturePreProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp,
	const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent)
{
	if (ShouldIgnoreEvent(InWheelEvent))
	{
		return true;
	}

	if (InWheelEvent.GetWheelDelta() == 0)
	{
		return true;
	}

	// Fire capture event
	const FKey WheelKey = InWheelEvent.GetWheelDelta() > 0 ? EKeys::MouseScrollUp : EKeys::MouseScrollDown;
	Capture(WheelKey);
	return true;
}

bool FAutoSettingsInputCapturePreProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp,
	const FPointerEvent& MouseEvent)
{
	// If neither mouse axis are allowed, then don't even bother considering them
	if (!IsKeyAllowed(EKeys::MouseX) && !IsKeyAllowed(EKeys::MouseY))
	{
		return true;
	}

	if (ShouldIgnoreEvent(MouseEvent))
	{
		return true;
	}

	const FVector2D Delta = MouseEvent.GetCursorDelta();

	AccumulatedMouseDelta += Delta;

	const float RequiredDelta = FMath::Max(MouseMoveCaptureDistance, 0.0f);

	if (AccumulatedMouseDelta.Size() <= RequiredDelta)
	{
		return true;
	}

	UE_LOG(LogAutoSettingsInput, Log, TEXT("BindCapturePrompt: Capture axis from mouse delta: %s"), *AccumulatedMouseDelta.ToString());

	FKey AxisKey;
	float AxisDelta;
	float AxisScale;
	if (FMath::Abs(AccumulatedMouseDelta.X) > FMath::Abs(AccumulatedMouseDelta.Y))
	{
		// X axis
		AxisKey = EKeys::MouseX;
		AxisDelta = AccumulatedMouseDelta.X;
		// For Mouse X, the sign of the mouse delta matches the sign that Unreal provides for the Mouse X input axis
		// +MouseDelta.X is Right, and +MouseX axis is Right
		AxisScale = AxisDelta >= 0.f ? 1.f : -1.f;
	}
	else
	{
		// Y axis
		AxisKey = EKeys::MouseY;
		AxisDelta = AccumulatedMouseDelta.Y;
		// Y is a bit different to X axis
		// For Mouse Y, the sign of the mouse delta is inverted from the sign that Unreal provides for Mouse Y input axis
		// +MouseDelta.Y is Down, and +MouseY axis is Up
		// Therefore, we invert here to get the "correct" scale to use for the Mouse Y input axis
		AxisScale = AxisDelta < 0.f ? 1.f : -1.f;
	}

	Capture(AxisKey, AxisScale);

	return true;
}

void FAutoSettingsInputCapturePreProcessor::Capture(const FKey& PrimaryKey, float AxisScale)
{
	if (ClearKeys.Contains(PrimaryKey))
	{
		UE_LOG(LogAutoSettingsInput, Log, TEXT("BindCapturePrompt: Clear key pressed: %s - Clearing"), *PrimaryKey.ToString());
		// Clear is a session command, so the owning widget decides how the mapping value is persisted.
		OnClearRequested.Broadcast();
		return;
	}

	if (EscapeKeys.Contains(PrimaryKey))
	{
		UE_LOG(LogAutoSettingsInput, Log, TEXT("BindCapturePrompt: Escape key pressed: %s - Cancelling"), *PrimaryKey.ToString());
		// Cancelling is not considered a capture attempt so we don't do a rejection
		Cancel();
		return;
	}

	FCapturedInput CapturedInput;
	CapturedInput.Key = PrimaryKey;
	CapturedInput.AxisScale = AxisScale;

	UE_LOG(LogAutoSettingsInput, Log, TEXT("BindCapturePrompt: Captured key: %s, AxisScale: %f"), *PrimaryKey.ToString(), AxisScale);

	ConfirmCapture(CapturedInput);
}

void FAutoSettingsInputCapturePreProcessor::Cancel()
{
	OnCancelled.Broadcast();
}

bool FAutoSettingsInputCapturePreProcessor::ShouldIgnoreEvent(const FInputEvent& InputEvent) const
{
	// Ignore input coming from a different player
	if (OwningLocalPlayer->GetControllerId() != InputEvent.GetUserIndex())
	{
		return true;
	}

	return false;
}

bool FAutoSettingsInputCapturePreProcessor::IsKeyAllowed(FKey Key) const
{
	if (IsKeyAllowedDelegate.IsBound())
	{
		if (!IsKeyAllowedDelegate.Execute(Key))
		{
			// Delegate disallowed key
			return false;
		}
	}

	return true;
}

void FAutoSettingsInputCapturePreProcessor::ConfirmCapture(FCapturedInput CapturedInput)
{
	OnKeySelected.Broadcast(CapturedInput);
}
