// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/ResolutionOptionSource.h"
#include "Misc/SettingOption.h"
#include "Kismet/KismetSystemLibrary.h"

/**
 * Generates resolution options by querying system-supported fullscreen resolutions.
 * Formats them for UI display ("1920 X 1080") and console commands ("1920x1080").
 */
TArray<FSettingOption> UResolutionOptionSource::GetOptions_Implementation() const
{
	// Query system for all supported fullscreen resolutions
	// This uses platform-specific APIs to discover hardware capabilities
	TArray<FIntPoint> Resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

	// Transform raw resolution data into user-friendly setting options
	TArray<FSettingOption> Result;
	for (const FIntPoint& Resolution : Resolutions)
	{
		// Convert resolution components to text for formatting
		const FText X = FText::FromString(FString::FromInt(Resolution.X));
		const FText Y = FText::FromString(FString::FromInt(Resolution.Y));

		// Create user-friendly display label with spacing and uppercase X
		const FText Label = FText::Format(FText::FromString("{0} X {1}"), X, Y);

		// Create compact console value format compatible with engine console commands
		const FString Value = FText::Format(FText::FromString("{0}x{1}"), X, Y).ToString();

		// Package as setting option for UI controls
		Result.Add(FSettingOption(Label, Value));
	}

	return Result;
}