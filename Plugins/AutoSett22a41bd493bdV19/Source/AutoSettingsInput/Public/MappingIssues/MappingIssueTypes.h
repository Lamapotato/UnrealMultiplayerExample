// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedActionKeyMapping.h"
#include "UObject/Object.h"
#include "Utility/InputMappingUtils.h"
#include "MappingIssueTypes.generated.h"

UENUM(BlueprintType)
enum class EMappingIssueType : uint8
{
	None,
	KeyAlreadyMappedToAction,
};

USTRUCT(BlueprintType)
struct FMappingIssue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Auto Settings")
	EMappingIssueType Type = EMappingIssueType::None;

	UPROPERTY(BlueprintReadWrite, Category = "Auto Settings")
	class UInputMappingContext* Context = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Auto Settings")
	FPlayerKeyMapping Mapping;
};

USTRUCT(BlueprintType)
struct FMappingAttemptInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Auto Settings")
	FKey DesiredKey;

	UPROPERTY(BlueprintReadWrite, Category = "Auto Settings")
	TArray<FMappingIssue> Issues;

	bool CanUnbindPreviousMappings() const;
};

UENUM(BlueprintType)
enum class EMappingIssueResolution : uint8
{
	Cancel,
	Replace,
	KeepAll
};