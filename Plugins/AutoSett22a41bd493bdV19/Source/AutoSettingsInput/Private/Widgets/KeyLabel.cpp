// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/KeyLabel.h"
#include "Engine/Texture.h"
#include "Logging/StructuredLog.h"
#include "AutoSettingsInputConfig.h"
#include "AutoSettingsInputLogs.h"

UKeyLabel::UKeyLabel(const FObjectInitializer& ObjectInitializer)
	: UUserWidget(ObjectInitializer)
{
	KeyInvalidText = NSLOCTEXT("AutoSettings", "KeyInvalid", "-");
}

void UKeyLabel::SetKey(const FKey& InKey)
{
	Key = InKey;
	UpdateKeyLabel();
}

FText UKeyLabel::GetDisplayName() const
{
	if (!LabelOverride.IsEmpty())
	{
		return LabelOverride;
	}

	if (!HasValidKey())
	{
		return KeyInvalidText;
	}

	return GetDefault<UAutoSettingsInputConfig>()->GetKeyFriendlyName(Key);
}

bool UKeyLabel::HasIcon() const
{
	return GetIconBrush().HasUObject();
}

bool UKeyLabel::HasValidKey() const
{
	return Key.IsValid();
}

FSlateBrush UKeyLabel::GetIconBrush_Implementation() const
{
	const UKeyIconSource* KeyIconSource = GetDefault<UAutoSettingsInputConfig>()->GetKeyIconSource();

	if (!KeyIconSource || !HasValidKey())
	{
		return {};
	}

	// Use the KeyIconSource to look up the icon for the key
	return KeyIconSource->GetIconBrushForKey(GetKey(), GetOwningLocalPlayer());
}

ESlateVisibility UKeyLabel::GetIconVisibility_Implementation() const
{
	return HasIcon() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}

ESlateVisibility UKeyLabel::GetDisplayNameVisibility_Implementation() const
{
	return HasIcon() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
}

void UKeyLabel::NativeConstruct()
{
	Super::NativeConstruct();

	const UKeyIconSource* KeyIconSource = GetDefault<UAutoSettingsInputConfig>()->GetKeyIconSource();

	if (!ensure(KeyIconSource))
	{
		UE_LOGFMT(LogAutoSettingsInput, Error, "Key Icon Source not specified in project settings");
		return;
	}

	FOnKeyIconsChanged Delegate;
	Delegate.BindDynamic(this, &ThisClass::OnKeyIconsChanged);

	KeyIconSource->RegisterIconsChangedCallback(GetOwningLocalPlayer(), Delegate);
}

void UKeyLabel::OnKeyIconsChanged()
{
	// Key icon may have changed based on global tags, so refresh
	UpdateKeyLabel();
}