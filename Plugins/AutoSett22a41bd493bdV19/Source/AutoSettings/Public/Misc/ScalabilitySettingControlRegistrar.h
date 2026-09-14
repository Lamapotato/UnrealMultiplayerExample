// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "ScalabilitySettingControlRegistrar.generated.h"

USTRUCT()
struct FScalabilityControlGroup
{
	GENERATED_BODY()

	TArray<FName> ControlledCVars;
};

/**
 * Subsystem responsible for registering control relationships between scalability settings,
 * by reading Unreal's scalability config to determine which CVars are controlled by each scalability group
 */
UCLASS()
class AUTOSETTINGS_API UScalabilitySettingControlRegistrar : public UEngineSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	UPROPERTY()
	bool bHasParsedScalabilityControlGroups = false;

	UPROPERTY()
	TMap<FName, FScalabilityControlGroup> ScalabilityControlGroups;

	void ParseScalabilityControlGroups();
};