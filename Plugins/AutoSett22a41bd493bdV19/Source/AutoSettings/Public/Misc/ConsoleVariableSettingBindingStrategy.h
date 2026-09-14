// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingBindingStrategy.h"

#include "ConsoleVariableSettingBindingStrategy.generated.h"

/**
 * Binding strategy that applies setting values to Unreal Engine console variables.
 */
UCLASS()
class AUTOSETTINGS_API UConsoleVariableSettingBindingStrategy : public USettingBindingStrategy
{
	GENERATED_BODY()

public:
	// Console variable to use for this setting type
	// If none, the leaf fragment of the Setting Key for each setting is used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	FName ConsoleVariableName = NAME_None;

	// Transformation to apply to the value to get the subsection that this setting cares about
	// Leave this empty for no transformation
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Setting")
	class USettingValueMask* ValueMask;

	UConsoleVariableSettingBindingStrategy();

	virtual void OnSyncToTarget_Implementation(const FSetSettingEvent& Event) override;
	virtual FString GetTargetValue_Implementation(const FSettingReference& Setting) override;
	virtual bool ValidateConfiguration_Implementation(const FSettingReference& Setting) override;
	virtual void BeginBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding) override;
	virtual void EndBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding) override;

	FName GetConsoleVariableName(const FFullSettingKey& Key) const;
};
