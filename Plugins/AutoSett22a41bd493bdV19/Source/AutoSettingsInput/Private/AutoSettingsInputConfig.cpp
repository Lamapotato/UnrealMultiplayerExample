// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsInputConfig.h"
#include "GameFramework/InputSettings.h"
#include "AutoSettingsInputLogs.h"
#include "Display/KeyIconSource.h"
#include "ConfigUtils.h"

UTexture* FKeyIconSet::GetIcon(const FKey& Key) const
{
	if (!IconMap.Contains(Key))
	{
		return nullptr;
	}

	UTexture* FoundIcon = IconMap.Find(Key)->LoadSynchronous();

	if (!FoundIcon)
	{
		UE_LOG(LogAutoSettingsInput, Warning, TEXT("FKeyIconSet::GetIcon: Key icon defined but empty - Key: %s, tags: %s"), *Key.ToString(), *Tags.ToString());
	}

	return FoundIcon;
}

UAutoSettingsInputConfig::UAutoSettingsInputConfig()
{
	CategoryName = "Plugins";

	// Auto-generate all axis associations including gamepad, VR, and other controllers
	AxisAssociations = GenerateAxisAssociations();

	// Migrate settings from old UAutoSettingsInputProjectConfig section
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		FConfigUtils::MigrateConfigPropertiesFromSection(this, {}, TEXT("/Script/AutoSettingsInput.AutoSettingsInputProjectConfig"));
		FConfigUtils::MigrateConfigPropertiesFromSection(this, {}, TEXT("/Script/AutoSettings.AutoSettingsConfig"));
	}
}

TArray<FAxisAssociation> UAutoSettingsInputConfig::GenerateAxisAssociations()
{
	TArray<FAxisAssociation> Associations;

	// Define axis patterns to check
	struct FAxisPattern
	{
		FString Suffix;
		FString ButtonFormat;
		bool bHasNegative;

		FAxisPattern(const FString& InSuffix, const FString& InButtonFormat, bool bInHasNegative = true)
			: Suffix(InSuffix), ButtonFormat(InButtonFormat), bHasNegative(bInHasNegative) {}
	};

	TArray<FAxisPattern> Patterns = {
		// X-axis patterns
		{ TEXT("X"), TEXT("%sStick_Right"), true },					  // Gamepad_LeftX -> Gamepad_LeftStick_Right/Left
		{ TEXT("_Thumbstick_X"), TEXT("%s_Thumbstick_Right"), true }, // VR thumbsticks
		{ TEXT("_Trackpad_X"), TEXT("%s_Trackpad_Right"), true },	  // VR trackpads

		// Y-axis patterns
		{ TEXT("Y"), TEXT("%sStick_Up"), true },				   // Gamepad_LeftY -> Gamepad_LeftStick_Up/Down
		{ TEXT("_Thumbstick_Y"), TEXT("%s_Thumbstick_Up"), true }, // VR thumbsticks
		{ TEXT("_Trackpad_Y"), TEXT("%s_Trackpad_Up"), true },	   // VR trackpads

		// Trigger patterns (single direction) - verified against EKeys
		{ TEXT("TriggerAxis"), TEXT("%sTrigger"), false },			// Gamepad_LeftTriggerAxis -> Gamepad_LeftTrigger
		{ TEXT("_TriggerAxis"), TEXT("%s_TriggerAxis"), false },	// MotionController_Left_TriggerAxis (no corresponding button)
		{ TEXT("_Trigger_Axis"), TEXT("%s_Trigger_Click"), false }, // MixedReality/OculusTouch/ValveIndex_*_Trigger_Axis -> *_Trigger_Click

		// Grip patterns (single direction) - verified against EKeys
		{ TEXT("_Grip_Axis"), TEXT("%s_Grip_Click"), false }, // OculusTouch/ValveIndex_*_Grip_Axis -> *_Grip_Click
	};

	// Get all keys from EKeys
	TArray<FKey> AllKeys;
	EKeys::GetAllKeys(AllKeys);

	for (const FKey& Key : AllKeys)
	{
		if (!Key.IsAxis1D())
		{
			continue;
		}

		const FString KeyName = Key.ToString();

		// Try each pattern
		for (const FAxisPattern& Pattern : Patterns)
		{
			if (KeyName.EndsWith(Pattern.Suffix))
			{
				const FString BaseName = KeyName.LeftChop(Pattern.Suffix.Len());

				if (Pattern.bHasNegative)
				{
					// Bidirectional axis (X/Y)
					FString PositiveKeyName = Pattern.ButtonFormat.Replace(TEXT("%s"), *BaseName);
					FKey PositiveKey = FKey(*PositiveKeyName);

					// Determine negative key name based on positive key
					FString NegativeFormat;
					if (Pattern.ButtonFormat.Contains(TEXT("Right")))
					{
						NegativeFormat = Pattern.ButtonFormat.Replace(TEXT("Right"), TEXT("Left"));
					}
					else if (Pattern.ButtonFormat.Contains(TEXT("Up")))
					{
						NegativeFormat = Pattern.ButtonFormat.Replace(TEXT("Up"), TEXT("Down"));
					}

					FString NegativeKeyName = NegativeFormat.Replace(TEXT("%s"), *BaseName);
					FKey NegativeKey = FKey(*NegativeKeyName);

					if (PositiveKey.IsValid() && NegativeKey.IsValid())
					{
						Associations.Add(FAxisAssociation(Key, { FKeyScale(PositiveKey, 1.f), FKeyScale(NegativeKey, -1.f) }));
						break; // Found a match, stop checking other patterns
					}
				}
				else
				{
					// Unidirectional axis (triggers, grips)
					FString PositiveKeyName = Pattern.ButtonFormat.Replace(TEXT("%s"), *BaseName);
					FKey PositiveKey = FKey(*PositiveKeyName);

					if (PositiveKey.IsValid())
					{
						Associations.Add(FAxisAssociation(Key, { FKeyScale(PositiveKey, 1.f) }));
						break; // Found a match, stop checking other patterns
					}
				}
			}
		}
	}

	return Associations;
}

UTexture* GetIcon(FKey Key, FKey AxisButtonKey, const FKeyIconSet& Set)
{
	// If the axis button key is valid, check it first
	if (AxisButtonKey.IsValid())
	{
		UTexture* AxisButtonTexture = Set.GetIcon(AxisButtonKey);
		if (AxisButtonTexture)
		{
			return AxisButtonTexture;
		}
	}

	return Set.GetIcon(Key);
}

const UKeyIconSource* UAutoSettingsInputConfig::GetKeyIconSource() const
{
	if (KeyIconSource.IsNull())
	{
		return nullptr;
	}

	const TSubclassOf<UKeyIconSource> Loaded = KeyIconSource.LoadSynchronous();
	if (!IsValid(Loaded))
	{
		return nullptr;
	}

	return GetDefault<UKeyIconSource>(Loaded);
}

UTexture* UAutoSettingsInputConfig::GetIconForKey(FKey InKey, FGameplayTagContainer Tags, float AxisScale) const
{
	UTexture* Result;

	// Retrieve axis button key if there is one, which may have a better icon
	const FKey AxisButton = GetAxisButton(InKey, AxisScale);

	// First check if an icon matches input tags exactly
	for (const FKeyIconSet& Set : KeyIconSets_DEPRECATED)
	{
		if (Set.Tags == Tags)
		{
			Result = GetIcon(InKey, AxisButton, Set);
			if (Result)
			{
				return Result;
			}
		}
	}

	// Check if an icon contains all input tags
	for (const FKeyIconSet& Set : KeyIconSets_DEPRECATED)
	{
		if (Set.Tags.HasAllExact(Tags))
		{
			Result = GetIcon(InKey, AxisButton, Set);
			if (Result)
			{
				return Result;
			}
		}
	}

	// Check if an icon contains any of the input tags
	for (const FKeyIconSet& Set : KeyIconSets_DEPRECATED)
	{
		if (Set.Tags.HasAnyExact(Tags))
		{
			Result = GetIcon(InKey, AxisButton, Set);
			if (Result)
			{
				return Result;
			}
		}
	}

	// Fall back to first icon we can find
	for (const FKeyIconSet& Set : KeyIconSets_DEPRECATED)
	{
		Result = GetIcon(InKey, AxisButton, Set);
		if (Result)
		{
			return Result;
		}
	}

	return nullptr;
}

FText UAutoSettingsInputConfig::GetKeyFriendlyName(FKey Key) const
{
	// There seems to be some issues where setting FKey of None in BP
	// then returns a display name of "(" and stuff like that, so just manually
	// handle case of none key
	if (!Key.IsValid())
	{
		return FText::FromString(FString(TEXT("None")));
	}

	for (const FKeyFriendlyName& KeyFriendlyName : KeyFriendlyNames)
	{
		if (KeyFriendlyName.Key == Key)
		{
			return KeyFriendlyName.FriendlyName;
		}
	}
	return Key.GetDisplayName();
}

const TArray<FKey>& UAutoSettingsInputConfig::GetDeprecatedBindingEscapeKeys() const
{
	// Widget classes seed their escape-key values from deprecated config data during construction.
	return BindingEscapeKeys_DEPRECATED;
}

float UAutoSettingsInputConfig::GetDeprecatedMouseMoveCaptureDistance() const
{
	// Widget classes seed their mouse-axis capture distance from deprecated config data during construction.
	return MouseMoveCaptureDistance_DEPRECATED;
}

const TArray<FKey>& UAutoSettingsInputConfig::GetDeprecatedAllowedKeys() const
{
	return AllowedKeys_DEPRECATED;
}

const TArray<FKey>& UAutoSettingsInputConfig::GetDeprecatedDisallowedKeys() const
{
	return DisallowedKeys_DEPRECATED;
}

FKeyScale UAutoSettingsInputConfig::GetAxisKey(FKey InButtonKey) const
{
	for (FAxisAssociation AxisAssociation : AxisAssociations)
	{
		for (FKeyScale ButtonKey : AxisAssociation.ButtonKeys)
		{
			if (ButtonKey.Key == InButtonKey)
			{
				return FKeyScale(AxisAssociation.AxisKey, ButtonKey.Scale);
			}
		}
	}

	return FKeyScale();
}

FKey UAutoSettingsInputConfig::GetAxisButton(FKey AxisKey, float AxisScale) const
{
	const FAxisAssociation* FoundAssociation = AxisAssociations.FindByPredicate([AxisKey](const FAxisAssociation Association)
		{
			return Association.AxisKey == AxisKey;
		});

	if (!FoundAssociation)
	{
		return EKeys::Invalid;
	}

	const FKeyScale* FoundKeyScale = FoundAssociation->ButtonKeys.FindByPredicate([AxisScale](const FKeyScale KeyScale)
		{
			return KeyScale.Scale == AxisScale;
		});

	if (!FoundKeyScale)
	{
		return EKeys::Invalid;
	}

	return FoundKeyScale->Key;
}

bool UAutoSettingsInputConfig::IsKeyAllowed(FKey Key) const
{
	if (AllowedKeys_DEPRECATED.Num() > 0)
	{
		bool bHasAllowedKey = false;

		for (FKey AllowedKey : AllowedKeys_DEPRECATED)
		{
			if (Key == AllowedKey)
			{
				bHasAllowedKey = true;
				break;
			}
		}

		if (!bHasAllowedKey)
		{
			// Whitelist populated but doesn't have key, therefore key is disallowed
			return false;
		}
	}

	// Disallow if key on blacklist

	for (FKey DisallowedKey : DisallowedKeys_DEPRECATED)
	{
		if (Key == DisallowedKey)
		{
			return false;
		}
	}

	// Passed both whitelist and blacklist, key is allowed
	return true;
}

bool UAutoSettingsInputConfig::IsAxisKey(FKey Key) const
{
	const FAxisAssociation* FoundAssociation = AxisAssociations.FindByPredicate([Key](const FAxisAssociation Association)
		{
			return Association.AxisKey == Key;
		});

	return FoundAssociation != nullptr;
}

FText UAutoSettingsInputConfig::GetKeyFriendlyNameStatic(FKey Key)
{
	return GetDefault<UAutoSettingsInputConfig>()->GetKeyFriendlyName(Key);
}

#if WITH_EDITOR
FText UAutoSettingsInputConfig::GetSectionText() const
{
	return INVTEXT("Auto Settings Input");
}

FText UAutoSettingsInputConfig::GetSectionDescription() const
{
	return INVTEXT("Configure the Auto Settings input mapping module");
}
#endif
