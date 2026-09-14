// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingKey.h"

#include "Framework/SettingType.h"

namespace
{
	// Using a function instead of a static variable avoids Live Coding corruption issues
	const TCHAR* GetSettingKeyDelimiter()
	{
		return TEXT(":");
	}
} // namespace

FSettingKey::FSettingKey() {}

FSettingKey::FSettingKey(const FString& FullName)
{
	TArray<FString> Parsed;
	FullName.ParseIntoArray(Parsed, GetSettingKeyDelimiter());
	for (const FString& Name : Parsed)
	{
		Fragments.Add(FSettingKeyFragment(FName(Name)));
	}
}

FString FSettingKey::ToString() const
{
	return FString::JoinBy(Fragments, GetSettingKeyDelimiter(), [](FSettingKeyFragment Fragment)
		{
			return Fragment.Name.ToString();
		});
}

FName FSettingKey::ToFullName() const
{
	return FName(ToString());
}

bool FSettingKey::IsChildOf(const FSettingKey& Parent) const
{
	if (Fragments.Num() <= Parent.Fragments.Num())
	{
		// The parent should have FEWER key names than the child
		return false;
	}

	for (int32 i = 0; i < Parent.Fragments.Num(); i++)
	{
		if (Fragments[i].Name != Parent.Fragments[i].Name)
		{
			return false;
		}
	}

	return true;
}

bool FSettingKey::EndsWith(const FSettingKey& Other) const
{
	// Iterate the other key fragments backward
	for (int32 i = 0; i < Other.Fragments.Num(); i++)
	{
		if (Other.Fragments.Last(i) != Fragments.Last(i))
		{
			return false;
		}
	}

	return true;
}

bool FSettingKey::IsValid() const
{
	if (Fragments.Num() == 0)
	{
		return false;
	}
	for (const FSettingKeyFragment& Fragment : Fragments)
	{
		if (Fragment.Name.IsNone())
		{
			return false;
		}
	}
	return true;
}

bool FSettingSpecifier::UsesSettingType() const
{
	return SettingType != nullptr;
}

bool FSettingSpecifier::IsSet() const
{
	return UsesSettingType() || SubKey.IsValid();
}

FPartialSettingKey FSettingSpecifier::ResolveKey() const
{
	if (UsesSettingType())
	{
		return SB::AutoSettings::SettingKeys::MakeSettingKey(SettingType, SubKey);
	}

	return SubKey;
}

FString FSettingSpecifier::ToString() const
{
	return ResolveKey().ToString();
}

namespace SB::AutoSettings::SettingKeys
{
	FPartialSettingKey MakeSettingKey(const TSubclassOf<::USettingType>& SettingType, const FPartialSettingKey& SubKey)
	{
		if (!SettingType)
		{
			return FPartialSettingKey{};
		}

		// Get the setting type's partial key (includes parent fragments)
		const ::USettingType* SettingAsset = GetDefault<::USettingType>(SettingType);
		FPartialSettingKey Key = SettingAsset->GetPartialSettingKey();

		// Add the sub-key if provided
		Key += SubKey;

		return Key;
	}
} // namespace SB::AutoSettings::SettingKeys
