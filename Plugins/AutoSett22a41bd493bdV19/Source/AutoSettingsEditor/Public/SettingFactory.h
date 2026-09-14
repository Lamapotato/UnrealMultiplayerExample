// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "SettingFactory.generated.h"

/**
 *
 */
UCLASS()
class AUTOSETTINGSEDITOR_API USettingFactory : public UFactory
{
	GENERATED_BODY()

	USettingFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	virtual FText GetDisplayName() const override;
};
