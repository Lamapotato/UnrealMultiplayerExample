// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Commands/InputChord.h"
#include "AutoSettingsInputCapturePreProcessor.generated.h"

struct FInputEvent;
class ULocalPlayer;

UENUM()
enum class EBindingCaptureMode : uint8
{
	// Capture keys when they are released (default)
	OnReleased,
	// Capture keys when they are pressed. This will prevent use of modifiers, since a modifier will be captured by itself when it is pressed.
	OnPressed,
};

USTRUCT(BlueprintType)
struct AUTOSETTINGSINPUT_API FCapturedInput
{
	GENERATED_BODY()

	/**
	 * Key that was captured
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FKey Key;

	/**
	 * Not currently used
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float AxisScale = 1.f;
};

/**
 * Slate input processor for capturing user input during key binding.
 * Integrates with Slate's input system to intercept and process input events.
 */
class FAutoSettingsInputCapturePreProcessor : public IInputProcessor
{
public:
	TWeakObjectPtr<ULocalPlayer> OwningLocalPlayer;

	EBindingCaptureMode CaptureMode = EBindingCaptureMode::OnReleased;

	// Keys that request capture cancellation for this capture session.
	TArray<FKey> EscapeKeys;

	// Keys that request a mapping clear for this capture session.
	TArray<FKey> ClearKeys;

	// Required mouse movement in pixels before mouse axes are captured for this session.
	float MouseMoveCaptureDistance = 80.f;

	DECLARE_MULTICAST_DELEGATE_OneParam(FInputCaptureKeySelected, FCapturedInput);
	FInputCaptureKeySelected OnKeySelected;

	DECLARE_MULTICAST_DELEGATE(FInputCaptureCancelled);
	FInputCaptureCancelled OnCancelled;

	DECLARE_MULTICAST_DELEGATE(FInputCaptureClearRequested);
	FInputCaptureClearRequested OnClearRequested;

	DECLARE_DELEGATE_RetVal_OneParam(bool, FInputCaptureIsKeyAllowed, FKey);
	FInputCaptureIsKeyAllowed IsKeyAllowedDelegate;

	FAutoSettingsInputCapturePreProcessor()
	{
	}

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override
	{
		return true;
	}

	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent,
		const FPointerEvent* InGestureEvent) override;

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

private:
	// Keys that are currently down, used to collect modifiers
	TArray<FKey> KeysDown = {};

	// Total delta the mouse has moved
	FVector2D AccumulatedMouseDelta = FVector2D::ZeroVector;

	void Capture(const FKey& PrimaryKey = EKeys::Invalid, float AxisScale = 1.f);

	void Cancel();

	bool ShouldIgnoreEvent(const FInputEvent& InputEvent) const;

	bool IsKeyAllowed(FKey Key) const;

	void ConfirmCapture(FCapturedInput CapturedInput);

	void RejectCapture(FCapturedInput CapturedInput);
};
