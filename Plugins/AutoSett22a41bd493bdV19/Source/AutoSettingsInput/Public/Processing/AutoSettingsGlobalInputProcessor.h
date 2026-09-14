// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"

/**
 * Input processor used to flip the Gamepad Right Thumbstick Y Axis back to the correct direction
 * This essentially undoes the logic in FSceneViewport::OnAnalogValueChanged which originally flips the axis
 */
class FAutoSettingsGlobalInputProcessor : public IInputProcessor
{

public:
	FAutoSettingsGlobalInputProcessor() {}

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
};

class FAutoSettingsGlobalInputProcessorRegistrar
{
public:
	FAutoSettingsGlobalInputProcessorRegistrar();
	~FAutoSettingsGlobalInputProcessorRegistrar();

private:
	TSharedPtr<FAutoSettingsGlobalInputProcessor> InputPreProcessor;
};