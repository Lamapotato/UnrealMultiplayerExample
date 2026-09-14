// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ResolutionStringUtils.generated.h"

/**
 * Helpers for dealing with strings combining resolution with window mode, for example "1920x1080wf"
 */
UCLASS()
class AUTOSETTINGS_API UResolutionStringUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Extracts the resolution portion from a combined resolution/window mode string.
	 * Example: "1920x1080wf" -> "1920x1080"
	 */
	static FString GetPixelsString(FString ResolutionString);

	/**
	 * Parses resolution string and returns pixel dimensions as IntPoint.
	 * Handles both pure resolution ("1920x1080") and combined format ("1920x1080wf").
	 */
	static FIntPoint GetPixels(FString ResolutionString);

	/**
	 * Extracts the window mode suffix from a resolution string.
	 * Example: "1920x1080wf" -> "wf" (windowed fullscreen)
	 */
	static FString GetMode(FString ResolutionString);

	/**
	 * Converts pixel dimensions to standard resolution string format.
	 * Example: FIntPoint(1920, 1080) -> "1920x1080"
	 */
	static FString PointToString(FIntPoint Pixels);
};