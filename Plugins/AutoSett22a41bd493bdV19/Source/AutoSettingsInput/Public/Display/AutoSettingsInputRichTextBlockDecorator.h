// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlock.h"
#include "Components/RichTextBlockDecorator.h"
#include "UObject/Object.h"
#include "Widgets/Layout/SScaleBox.h"
#include "AutoSettingsInputRichTextBlockDecorator.generated.h"

class URichTextBlockInputDecorator;
class FInlineInput;

typedef TVariant<FName, FKey> FInlineInputKeySource;

class SInlineInput : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInlineInput)
	{
	}
	SLATE_ATTRIBUTE(FInlineInputKeySource, KeySource)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const ULocalPlayer* LocalPlayer, const FTextBlockStyle& TextStyle, TOptional<int32> Width, TOptional<int32> Height, EStretch::Type Stretch);

private:
	FSlateBrush Brush;
};

/**
 * Add an image inline with the text.
 * Usage: Before image <img id="MyId"/>, after image.
 *
 * A width and height can be specified.
 * By default the width and the height is the same size as the font height.
 * Use "desired" to use the same size as the image brush.
 * Usage: Before image <img id="MyId" height="40" width="desired"/>, after image.
 *
 * A stretch type can be specified. See EStretch.
 * By default the stretch type is ScaleToFit.
 * Usage: Before image <img id="MyId" stretch="ScaleToFitY"/>, after image.
 */
class FInlineInput : public FRichTextDecorator
{
public:
	FInlineInput(URichTextBlock* InOwner, URichTextBlockInputDecorator* InDecorator)
		: FRichTextDecorator(InOwner)
		, Decorator(InDecorator)
	{
	}

	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override;

	ULocalPlayer* GetLocalPlayer() const { return Owner->GetOwningLocalPlayer(); }

protected:
	virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& TextStyle) const override;

private:
	URichTextBlockInputDecorator* Decorator;
};

/**
 *
 */
UCLASS()
class AUTOSETTINGSINPUT_API URichTextBlockInputDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()
public:
	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;
};
