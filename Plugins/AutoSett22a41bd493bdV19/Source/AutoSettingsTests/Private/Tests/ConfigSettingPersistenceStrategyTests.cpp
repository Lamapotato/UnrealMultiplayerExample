// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/ConfigSettingPersistenceStrategy.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"
#include "Framework/SettingTypes.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

TEST_CLASS(ConfigSettingPersistenceStrategy, "SB.AutoSettings.Persistence")
{
	TEST_METHOD(LoadsExistingUserSettingsFile)
	{
		// Use a unique user config name so the test never reads or writes the project's real UserSettings.ini file.
		const FString IniName = FString::Printf(TEXT("AutoSettingsUserPersistenceTest_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		const FString SectionName = TEXT("Settings");
		const FString KeyName = TEXT("AutomationPersistenceValue");
		const FString ExpectedValue = TEXT("LoadedExistingValue");

		// Prime Unreal's generated config branch to match a runtime where the global config cache already knows this user ini.
		FString IniPath;
		FConfigCacheIni::LoadGlobalIniFile(IniPath, *IniName);
		if (!TestRunner->AddErrorIfFalse(!IniPath.IsEmpty(), TEXT("Failed to resolve the generated config path for the test user ini.")))
		{
			return;
		}

		// Clean both the in-memory branch and generated file so the unique test config leaves no reusable state behind.
		TestCommandBuilder.OnTearDown([IniPath]()
			{
				if (GConfig)
				{
					GConfig->UnloadFile(IniPath);
				}

				IFileManager::Get().Delete(*IniPath, false, true, true);
			});

		// Write the saved value after the config branch is registered so the strategy must read the existing disk file.
		const FString IniDirectory = FPaths::GetPath(IniPath);
		const bool bDirectoryReady = IFileManager::Get().DirectoryExists(*IniDirectory) || IFileManager::Get().MakeDirectory(*IniDirectory, true);
		if (!TestRunner->AddErrorIfFalse(bDirectoryReady, *FString::Printf(TEXT("Failed to create generated config directory '%s'."), *IniDirectory)))
		{
			return;
		}

		// Seed the exact config shape that the persistence strategy reads through its public API.
		const FString IniContents = FString::Printf(TEXT("[%s]\n%s=%s\n"), *SectionName, *KeyName, *ExpectedValue);
		if (!TestRunner->AddErrorIfFalse(FFileHelper::SaveStringToFile(IniContents, *IniPath), *FString::Printf(TEXT("Failed to write test config file '%s'."), *IniPath)))
		{
			return;
		}

		// Create the concrete persistence strategy with the same config identity as the seeded file.
		TStrongObjectPtr<UConfigSettingPersistenceStrategy> Strategy(NewObject<UConfigSettingPersistenceStrategy>(GetTransientPackage()));
		if (!TestRunner->AddErrorIfFalse(Strategy.IsValid(), TEXT("Failed to create the config persistence strategy.")))
		{
			return;
		}

		Strategy->IniName = IniName;
		Strategy->SectionName = SectionName;

		// Read through the public persistence contract instead of reaching into config internals.
		FSettingReference SettingReference;
		SettingReference.Key = FFullSettingKey(KeyName);
		const FSettingData LoadedValue = Strategy->GetSavedValue(SettingReference);

		// The saved value must come from the existing generated user config file.
		TestRunner->AddErrorIfFalse(
			LoadedValue.Value == ExpectedValue,
			*FString::Printf(TEXT("Expected loaded value '%s', but got '%s'."), *ExpectedValue, *LoadedValue.Value));
	}
};

#endif
