// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsEditorSubsystem.h"

#include "Framework/SettingRegistry.h"
#include "Utility/SettingRegistryKeyUtils.h"
#include "Widgets/AutoSettingWidget.h"
#include "Utility/SettingUtils.h"

void UAutoSettingsEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAutoSettingWidget::GetDesignTimeRegistry.BindWeakLambda(this, [this](ESettingDomain Domain)
		{
			USettingRegistry*& Registry = Domain == ESettingDomain::Global ? DesignTimeGlobalRegistry : DesignTimePlayerRegistry;

			if (!Registry)
			{
				// Create the design-time registry for the requested domain the first time a widget asks for it.
				Registry = NewObject<USettingRegistry>();
				Registry->bDesignTime = true;
				Registry->BaseKey = Domain == ESettingDomain::Player
					? SB::AutoSettings::SettingRegistries::GetBaseKey(Domain, TEXT("0"))
					: SB::AutoSettings::SettingRegistries::GetBaseKey(Domain);

				// Configure the design-time registry with the correct domain assets first, then run the normal
				// initialization pipeline so widgets can read resolved defaults through the same registry path
				// they use at runtime.
				const TArray<USettingType*> Settings = FSettingUtils::GetAllSettingTypes(Domain);

				Registry->SetSettingAssets(Settings);
				Registry->Initialize();
			}

			return Registry;
		});
}
