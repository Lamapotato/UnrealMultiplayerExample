// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingPersistenceStrategy.h"

#include "ConfigSettingPersistenceStrategy.generated.h"

/**
 *
 */
UCLASS()
class AUTOSETTINGS_API UConfigSettingPersistenceStrategy : public USettingPersistenceStrategy
{
	GENERATED_BODY()

public:
	/**
	 * Ini file name to store the settings in
	 * This must contain the string "User" for Unreal to be able to save it from a package
	 */
	UPROPERTY()
	FString IniName;

	/**
	 * Section name to store the settings in
	 */
	UPROPERTY()
	FString SectionName;

	UConfigSettingPersistenceStrategy();

	virtual void OnSave_Implementation(const FSetSettingEvent& Event) override;
	virtual FSettingData GetSavedValue_Implementation(const FSettingReference& Setting) const override;
	virtual TArray<FFullSettingKey> FindKeys_Implementation() const override;

protected:
	FString GetSectionName() const;

private:
	FString GetIniPath() const;

	void SetConfigValue(FName Key, const FSettingData& Value);
	FSettingData GetConfigValue(FName Key) const;

	const FConfigSection* GetSection() const;
};