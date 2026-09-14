// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingTypes.h"

FString FSettingData::ExportString() const
{
	// FString String;
	// StaticStruct()->ExportText(String, this, nullptr, nullptr, PPF_None, nullptr);
	// return String;
	return Value;
}

FSettingData FSettingData::ImportString(FString String)
{
	FSettingData Value;
	// const TCHAR* Result = StaticStruct()->ImportText(*String, &Value, nullptr, PPF_None, GLog, StaticStruct()->GetName());
	// if (!Result)
	// {
	// 	// If failed to import struct, fall back to import string
	// 	Value.Value = String;
	// }
	Value.Value = String;
	return Value;
}

USettingRegistry* FSetSettingEvent::GetRegistry() const
{
	return Setting.Context.Registry;
}