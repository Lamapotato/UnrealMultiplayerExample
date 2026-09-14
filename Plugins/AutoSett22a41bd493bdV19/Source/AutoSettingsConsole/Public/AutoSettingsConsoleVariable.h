// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AutoSettingsConsoleVariable.generated.h"

/**
 * Class used to represent a Console Variable
 * An instance of each class is created for each GameInstance for the purpose of receiving events, but only one native Console Variable is created at the engine level
 */
UCLASS(Abstract)
class AUTOSETTINGSCONSOLE_API UAutoSettingsConsoleVariable : public UObject
{
	GENERATED_BODY()

public:
	inline const static FName ConsoleVariablePrimaryAssetTypeName = TEXT("ConsoleVariable");

	/**
	 * Name of the console variable
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Auto Settings")
	FName Name;

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// UConsoleVariableType* Type;

	/**
	 * Help text to describe what the console variable does and how to use it
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Auto Settings")
	FString Help;

	/**
	 * If true, the OnChanged function will be called immediately with the default value upon registration
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Auto Settings")
	bool bCallOnChangedWhenRegistered = true;

	/**
	 * Create only the native console variable with default value (no callbacks)
	 * Used by engine subsystem for engine-level CVar creation
	 */
	IConsoleVariable* CreateConsoleVariable();

	/**
	 * Register only the changed callback (assumes native console variable already exists)
	 * Used by game instance subsystem for per-instance callback registration
	 */
	void RegisterCallback();

protected:
	virtual UWorld* GetWorld() const override;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	virtual IConsoleVariable* RegisterConsoleVariable() PURE_VIRTUAL(ThisClass::RegisterConsoleVariable, return nullptr;);
	virtual void RegisterChangedCallback() PURE_VIRTUAL(ThisClass : RegisterChangedCallback, );
};

UCLASS(Abstract, Blueprintable)
class AUTOSETTINGSCONSOLE_API UConsoleVariable_Boolean : public UAutoSettingsConsoleVariable
{
	GENERATED_BODY()

public:
	/**
	 * Initial value to apply by default to the console variable
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Auto Settings")
	bool DefaultValue;

	// Public wrapper for callback access
	void CallOnChanged(bool NewValue) { OnChanged(NewValue); }

protected:
	virtual IConsoleVariable* RegisterConsoleVariable() override;
	virtual void RegisterChangedCallback() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnChanged(bool NewValue);
};

UCLASS(Abstract, Blueprintable)
class AUTOSETTINGSCONSOLE_API UConsoleVariable_Integer : public UAutoSettingsConsoleVariable
{
	GENERATED_BODY()

public:
	/**
	 * Initial value to apply by default to the console variable
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Auto Settings")
	int32 DefaultValue;

	// Public wrapper for callback access
	void CallOnChanged(int32 NewValue) { OnChanged(NewValue); }

protected:
	virtual IConsoleVariable* RegisterConsoleVariable() override;
	virtual void RegisterChangedCallback() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnChanged(int32 NewValue);
};

UCLASS(Abstract, Blueprintable)
class AUTOSETTINGSCONSOLE_API UConsoleVariable_Float : public UAutoSettingsConsoleVariable
{
	GENERATED_BODY()

public:
	/**
	 * Initial value to apply by default to the console variable
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Auto Settings")
	float DefaultValue;

	// Public wrapper for callback access
	void CallOnChanged(float NewValue) { OnChanged(NewValue); }

protected:
	virtual IConsoleVariable* RegisterConsoleVariable() override;
	virtual void RegisterChangedCallback() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnChanged(float NewValue);
};

UCLASS(Abstract, Blueprintable)
class AUTOSETTINGSCONSOLE_API UConsoleVariable_String : public UAutoSettingsConsoleVariable
{
	GENERATED_BODY()

public:
	/**
	 * Initial value to apply by default to the console variable
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Auto Settings")
	FString DefaultValue;

	// Public wrapper for callback access
	void CallOnChanged(const FString& NewValue) { OnChanged(NewValue); }

protected:
	virtual IConsoleVariable* RegisterConsoleVariable() override;
	virtual void RegisterChangedCallback() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnChanged(const FString& NewValue);
};