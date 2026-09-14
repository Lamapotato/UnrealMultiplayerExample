// Copyright Sam Bonifacio. All Rights Reserved.

#include "Display/AutoSettingsInputRichTextBlockDecorator.h"

#include "InputAction.h"
#include "Components/RichTextBlock.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "AutoSettingsInputConfig.h"
#include "Misc/DefaultValueHelper.h"
#include "Display/KeyIconSource.h"
#include "Utility/InputMappingUtils.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Text/STextBlock.h"

void SInlineInput::Construct(const FArguments& InArgs, const ULocalPlayer* LocalPlayer, const FTextBlockStyle& TextStyle, TOptional<int32> Width, TOptional<int32> Height, EStretch::Type Stretch)
{
	auto KeySource = InArgs._KeySource.Get();

	TArray<FKey> InputKeys;

	if (KeySource.IsType<FName>())
	{
		const FName ActionName = KeySource.Get<FName>();
		const UInputAction* InputAction = SB::AutoSettings::Input::Action::Find(ActionName);

		if (IsValid(InputAction))
		{
			InputKeys = LocalPlayer ? SB::AutoSettings::Input::Player::GetCurrentKeysForAction(LocalPlayer, InputAction) : SB::AutoSettings::Input::Action::GetDefaultKeys(InputAction);
		}
		else
		{
			const FKey CurrentKey = LocalPlayer ? SB::AutoSettings::Input::Player::GetCurrentKeyForMapping(LocalPlayer, ActionName) : SB::AutoSettings::Input::PlayerMapping::GetDefaultKey(LocalPlayer, ActionName);
			InputKeys.Add(CurrentKey);
		}
	}
	else
	{
		ensure(KeySource.IsType<FKey>());
		InputKeys.Add(KeySource.Get<FKey>());
	}

	const UKeyIconSource* KeyIconSource = GetDefault<UAutoSettingsInputConfig>()->GetKeyIconSource();

	// Determine the key to display. An empty key list means the action is unbound,
	// in which case we render a sentinel instead of an icon or key name.
	const FKey KeyToDisplay = InputKeys.Num() > 0 ? InputKeys[0] : FKey(EKeys::Invalid);
	const bool bHasValidKey = KeyToDisplay.IsValid();

	// Only look up an icon when we have both a valid key and a configured icon source.
	// Missing either is an expected fallback case; we render text instead.
	if (bHasValidKey && KeyIconSource)
	{
		Brush = KeyIconSource->GetIconBrushForKey(KeyToDisplay, LocalPlayer);
	}

	if (Brush.HasUObject())
	{
		const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		float IconHeight = FMath::Min((float)FontMeasure->GetMaxCharacterHeight(TextStyle.Font, 1.0f), Brush.ImageSize.Y);

		if (Height.IsSet())
		{
			IconHeight = Height.GetValue();
		}

		float IconWidth = IconHeight;
		if (Width.IsSet())
		{
			IconWidth = Width.GetValue();
		}

		ChildSlot
			[SNew(SBox)
					.HeightOverride(IconHeight)
					.WidthOverride(IconWidth)
						[SNew(SScaleBox)
								.Stretch(Stretch)
								.StretchDirection(EStretchDirection::DownOnly)
								.VAlign(VAlign_Center)
									[SNew(SImage)
											.Image(&Brush)]]];
	}
	else
	{
		const FText DisplayText = bHasValidKey
			? GetDefault<UAutoSettingsInputConfig>()->GetKeyFriendlyName(KeyToDisplay)
			: FText::FromString(TEXT("-"));

		ChildSlot
			[SNew(STextBlock)
					.TextStyle(&TextStyle)
					.Text(DisplayText)];
	}
}

bool FInlineInput::Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const
{
	// Inline input widgets are created only for input runs that name an action or a key.
	if (RunParseResult.Name == TEXT("input") && (RunParseResult.MetaData.Contains(TEXT("action")) || RunParseResult.MetaData.Contains(TEXT("key"))))
	{
		return true;
	}

	return false;
}

TSharedPtr<SWidget> FInlineInput::CreateDecoratorWidget(const FTextRunInfo& RunInfo,
	const FTextBlockStyle& TextStyle) const
{
	const bool bWarnIfMissing = true;

	FInlineInputKeySource KeySource;

	if (RunInfo.MetaData.Contains(TEXT("action")))
	{
		KeySource.Set<FName>(FName(RunInfo.MetaData[TEXT("action")]));
	}
	else
	{
		KeySource.Set<FKey>(FKey(FName(RunInfo.MetaData[TEXT("key")])));
	}

	// const FSlateBrush* Brush = Decorator->FindImageBrush(, bWarnIfMissing);

	// if (ensure(Brush))
	{
		TOptional<int32> Width;
		if (const FString* WidthString = RunInfo.MetaData.Find(TEXT("width")))
		{
			int32 WidthTemp;
			if (FDefaultValueHelper::ParseInt(*WidthString, WidthTemp))
			{
				Width = WidthTemp;
			}
			// else
			// {
			// 	if (FCString::Stricmp(GetData(*WidthString), TEXT("desired")) == 0)
			// 	{
			// 		Width = Brush->ImageSize.X;
			// 	}
			// }
		}

		TOptional<int32> Height;
		if (const FString* HeightString = RunInfo.MetaData.Find(TEXT("height")))
		{
			int32 HeightTemp;
			if (FDefaultValueHelper::ParseInt(*HeightString, HeightTemp))
			{
				Height = HeightTemp;
			}
			// else
			// {
			// 	if (FCString::Stricmp(GetData(*HeightString), TEXT("desired")) == 0)
			// 	{
			// 		Height = Brush->ImageSize.Y;
			// 	}
			// }
		}

		EStretch::Type Stretch = EStretch::ScaleToFit;
		if (const FString* SstretchString = RunInfo.MetaData.Find(TEXT("stretch")))
		{
			const UEnum* StretchEnum = StaticEnum<EStretch::Type>();
			int64 StretchValue = StretchEnum->GetValueByNameString(*SstretchString);
			if (StretchValue != INDEX_NONE)
			{
				Stretch = static_cast<EStretch::Type>(StretchValue);
			}
		}

		const ULocalPlayer* LocalPlayer = Owner->GetOwningLocalPlayer();

		return SNew(SInlineInput, LocalPlayer, TextStyle, Width, Height, Stretch)
			.KeySource(KeySource);
	}
	// return TSharedPtr<SWidget>();
}

TSharedPtr<ITextDecorator> URichTextBlockInputDecorator::CreateDecorator(URichTextBlock* InOwner)
{
	return MakeShareable(new FInlineInput(InOwner, this));
}
