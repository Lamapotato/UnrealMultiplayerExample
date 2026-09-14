// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Processing/AutoSettingsInputCapturePreProcessor.h"
#include "Widgets/AutoSettingWidget.h"
#include "MappingIssues/MappingIssueTypes.h"
#include "AutoSettingsInputConfig.h"
#include "InputMappingSettingWidget.generated.h"

class UInputKeyFilter;
struct FMappingAttemptInfo;
class UInputMappingContext;
class UActionLabel;
enum class EBindingCaptureMode : uint8;

/**
 * How to handle a rejected input key during an active capture session.
 */
UENUM(BlueprintType)
enum class EInputCaptureRejectedAction : uint8
{
	/** Stops listening and ends capture on this key. */
	StopCapturing,

	/** Keeps listening for a valid key while ignoring this key as a binding candidate. */
	KeepCapturing,
};

/**
 * Widget that can be used to display an input mapping and allow the player to rebind it
 */
UCLASS(abstract)
class AUTOSETTINGSINPUT_API UInputMappingSettingWidget : public UAutoSettingWidget
{
	GENERATED_BODY()

public:
	UInputMappingSettingWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Optional key filter adapter. If set, only keys matching this filter are considered for binding.
	 */
	UPROPERTY(EditAnywhere, Category = "Setting|Capture|Filtering")
	UInputKeyFilter* KeyFilter;

	/** Key whitelist, use to allow specific keys for input mapping
	 * Leave empty to allow all keys
	 * Keys here will have no effect if also in DisallowedKeys
	 * If not set, seeded from deprecated UAutoSettingsInputConfig values
	 */
	UPROPERTY(EditAnywhere, Category = "Setting|Capture|Filtering")
	TArray<FKey> AllowedKeys;

	/**
	 * Key blacklist, use to disallow specific keys for input mapping
	 * Will take precedence over AllowedKeys
	 * If not set, seeded from deprecated UAutoSettingsInputConfig values
	 */
	UPROPERTY(EditAnywhere, Category = "Setting|Capture|Filtering")
	TArray<FKey> DisallowedKeys;

	/**
	 * Should keys and mouse clicks be captured when they are pressed, or when they are released?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Capture")
	EBindingCaptureMode CaptureMode;

	/**
	 * Distance in pixels the mouse must travel before this widget captures a mouse axis.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Capture")
	float MouseMoveCaptureDistance = 80.f;

	/**
	 * Keys that cancel this widget's active capture without changing its mapping.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Capture")
	TArray<FKey> EscapeKeys;

	/**
	 * Keys that clear this widget's mapping while it is capturing input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Capture")
	TArray<FKey> ClearKeys;

	/**
	 * Controls whether a rejected key ends the capture session.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Capture|Filtering")
	EInputCaptureRejectedAction CaptureRejectedAction = EInputCaptureRejectedAction::StopCapturing;

	/** Fired when an input chord is captured using this button. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCapturingChangedEvent, bool, IsCapturing);

	UPROPERTY(BlueprintAssignable, Category = "Setting|Events")
	FCapturingChangedEvent OnCapturingChanged;

	// Fired when an input chord is captured using this button
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChordCapturedEvent, FCapturedInput, CapturedInput);

	UPROPERTY(BlueprintAssignable, Category = "Setting|Events")
	FChordCapturedEvent OnChordCaptured;

	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Setting|Actions")
	void SetMapping(FName MappingName, EPlayerMappableKeySlot KeySlot);

	void UpdateSettingKey();

	/** Create prompt and start listening for input chord. */
	UFUNCTION(BlueprintCallable, Category = "Setting|Actions")
	void StartCapture();

	/** Stop listening for input events. */
	UFUNCTION(BlueprintCallable, Category = "Setting|Actions")
	void StopCapture();

	/** Clear this mapping so it will not fall back to its default key. */
	UFUNCTION(BlueprintCallable, Category = "Setting|Actions")
	void ClearMapping();

	UFUNCTION(BlueprintPure, Category = "Setting|State")
	bool IsCapturing() const;

protected:
	// Name of the action mapping to display and rebind
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Mapping")
	FName MappingName;

	// Which slot of the action mapping to display and rebind
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Mapping")
	EPlayerMappableKeySlot KeySlot;

	virtual void UpdateSelection_Implementation(const FString& Value) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Setting|Overrides")
	void OnKeyChanged(const FKey& Key);

	// Returns true if the key is allowed to be selected with this selector.
	UFUNCTION(BlueprintNativeEvent, Category = "Setting|Overrides")
	bool IsKeyAllowed(const FKey& PrimaryKey);

	/**
	 * Returns the capture-session action for a key rejected by IsKeyAllowed.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Setting|Overrides")
	EInputCaptureRejectedAction GetCaptureRejectedAction(const FKey& PrimaryKey) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Setting|Overrides")
	void OnEncounteredMappingIssue(const FMappingAttemptInfo& Issues);

	UFUNCTION(BlueprintCallable, Category = "Setting|Resolution")
	void SetMappingIssueResolution(EMappingIssueResolution Resolution);
	virtual void PostLoad() override;

	// #if WITH_EDITOR
	// 	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// #endif

	virtual TFuture<EMappingIssueResolution> DetermineResolutionAsync(const FMappingAttemptInfo& Issues);

private:
	TSharedPtr<class FAutoSettingsInputCapturePreProcessor> InputProcessor;

	TSharedPtr<TPromise<EMappingIssueResolution>> InProgressIssueResolutionPromise;

	/** Normalize a raw captured input before binding (button -> axis if mapped). */
	FCapturedInput ResolveCapturedInput(const FCapturedInput& CapturedInput) const;

	void StoreMappingDirectionalIntent(const FCapturedInput& CapturedInput);
	void BindKey(const FCapturedInput& CapturedInput);
};
