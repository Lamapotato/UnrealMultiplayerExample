// Copyright Sam Bonifacio. All Rights Reserved.

#include "Processing/AutoSettingsGlobalInputProcessor.h"

#include "Processing/AutoSettingsInputCapturePreProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "AutoSettingsInputConfig.h"

bool FAutoSettingsGlobalInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp,
	const FAnalogInputEvent& InAnalogInputEvent)
{
	if (InAnalogInputEvent.GetKey() == EKeys::Gamepad_RightY)
	{
		const auto InputConfig = GetDefault<UAutoSettingsInputConfig>();
		const bool bShouldUninvert = InputConfig->bUninvertGamepadRightStickYAxis;
		if (bShouldUninvert)
		{
			// Negate the value of the thumbstick axis to fix the FSceneViewport::OnAnalogValueChanged logic
			// Construct a new analog input event with the negated value and replace the original one
			FAnalogInputEvent* MutableEvent = const_cast<FAnalogInputEvent*>(&InAnalogInputEvent);
			const FAnalogInputEvent NewEvent(
				InAnalogInputEvent.GetKey(),
				InAnalogInputEvent.GetModifierKeys(),
				InAnalogInputEvent.GetInputDeviceId(),
				InAnalogInputEvent.IsRepeat(),
				InAnalogInputEvent.GetCharacter(),
				InAnalogInputEvent.GetKeyCode(),
				InAnalogInputEvent.GetAnalogValue() * -1.f,
				InAnalogInputEvent.GetUserIndex());
			*MutableEvent = NewEvent;
		}
	}
	return false;
}

FAutoSettingsGlobalInputProcessorRegistrar::FAutoSettingsGlobalInputProcessorRegistrar()
{
	if (!FSlateApplication::IsInitialized())
	{
		// Skip in commandlet and dedicated server
		return;
	}

	InputPreProcessor = MakeShared<FAutoSettingsGlobalInputProcessor>();
	FSlateApplication::Get().RegisterInputPreProcessor(InputPreProcessor, 0);
}

FAutoSettingsGlobalInputProcessorRegistrar::~FAutoSettingsGlobalInputProcessorRegistrar()
{
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}
	FSlateApplication::Get().UnregisterInputPreProcessor(InputPreProcessor);
}