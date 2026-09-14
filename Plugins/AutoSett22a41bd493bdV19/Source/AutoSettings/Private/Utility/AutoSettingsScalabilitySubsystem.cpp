// Copyright Sam Bonifacio. All Rights Reserved.

#include "Utility/AutoSettingsScalabilitySubsystem.h"
#include "AutoSettingsLogs.h"
#include "Scalability.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "Logging/StructuredLog.h"

static TAutoConsoleVariable<bool> CVarDebugRandomBenchmarkValues(
	TEXT("AutoSettings.Scalability.DebugRandomBenchmarkValues"),
	false,
	TEXT("If true, use random values instead of running actual hardware benchmark for testing purposes"),
	ECVF_Default);

static FAutoConsoleCommandWithWorldAndArgs ClearBenchmarkCommand(
	TEXT("AutoSettings.Scalability.ClearCachedBenchmarkResults"),
	TEXT("Clear cached benchmark results from the scalability subsystem"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (!GEngine)
			{
				UE_LOG(LogAutoSettings, Warning, TEXT("Engine not available"));
				return;
			}

			UAutoSettingsScalabilitySubsystem* ScalabilitySubsystem = GEngine->GetEngineSubsystem<UAutoSettingsScalabilitySubsystem>();
			if (!ScalabilitySubsystem)
			{
				UE_LOG(LogAutoSettings, Warning, TEXT("AutoSettings Scalability Subsystem not found"));
				return;
			}

			ScalabilitySubsystem->ClearCachedResults();
			UE_LOG(LogAutoSettings, Log, TEXT("Benchmark cache cleared via console command"));
		}));

void UAutoSettingsScalabilitySubsystem::RunBenchmark(int32 WorkScale, float CPUMultiplier, float GPUMultiplier)
{
	UE_LOGFMT(LogAutoSettings, Log, "AutoSettingsScalabilitySubsystem: Running hardware benchmark (WorkScale={0}, CPU={1}, GPU={2})",
		WorkScale, CPUMultiplier, GPUMultiplier);

	// Run the scalability benchmark or generate debug values
	Scalability::FQualityLevels QualityLevels;

	if (CVarDebugRandomBenchmarkValues.GetValueOnGameThread())
	{
		// Generate random values for testing
		UE_LOGFMT(LogAutoSettings, Warning, "AutoSettingsScalabilitySubsystem: Using debug random benchmark values instead of real benchmark");
		QualityLevels.ResolutionQuality = FMath::RandRange(50, 100); // ResolutionQuality is percentage-based
		QualityLevels.ViewDistanceQuality = FMath::RandRange(0, 3);
		QualityLevels.AntiAliasingQuality = FMath::RandRange(0, 3);
		QualityLevels.ShadowQuality = FMath::RandRange(0, 3);
		QualityLevels.GlobalIlluminationQuality = FMath::RandRange(0, 3);
		QualityLevels.ReflectionQuality = FMath::RandRange(0, 3);
		QualityLevels.PostProcessQuality = FMath::RandRange(0, 3);
		QualityLevels.TextureQuality = FMath::RandRange(0, 3);
		QualityLevels.EffectsQuality = FMath::RandRange(0, 3);
		QualityLevels.FoliageQuality = FMath::RandRange(0, 3);
		QualityLevels.ShadingQuality = FMath::RandRange(0, 3);
	}
	else
	{
		// Run actual hardware benchmark
		QualityLevels = Scalability::BenchmarkQualityLevels(WorkScale, CPUMultiplier, GPUMultiplier);
	}

	// Cache the results
	CachedResults = QualityLevels;

	UE_LOGFMT(LogAutoSettings, Log, "AutoSettingsScalabilitySubsystem: Benchmark completed - Overall={0}, View={1}, Shadow={2}, GlobalIllumination={3}, Reflection={4}, AntiAliasing={5}, Texture={6}, Effects={7}, Foliage={8}, Shading={9}, PostProcess={10}",
		QualityLevels.GetSingleQualityLevel(), QualityLevels.ViewDistanceQuality, QualityLevels.ShadowQuality,
		QualityLevels.GlobalIlluminationQuality, QualityLevels.ReflectionQuality, QualityLevels.AntiAliasingQuality,
		QualityLevels.TextureQuality, QualityLevels.EffectsQuality, QualityLevels.FoliageQuality,
		QualityLevels.ShadingQuality, QualityLevels.PostProcessQuality);
}

bool UAutoSettingsScalabilitySubsystem::IsBenchmarkDataAvailable() const
{
	return CachedResults.IsSet();
}

void UAutoSettingsScalabilitySubsystem::ClearCachedResults()
{
	CachedResults.Reset();

	UE_LOGFMT(LogAutoSettings, Log, "AutoSettingsScalabilitySubsystem: Cached benchmark results cleared");
}

TOptional<Scalability::FQualityLevels> UAutoSettingsScalabilitySubsystem::GetCachedResults() const
{
	return CachedResults;
}

void UAutoSettingsScalabilitySubsystem::SetQualityFromBenchmark(bool bForceRunBenchmark)
{
	UE_LOGFMT(LogAutoSettings, Log, "AutoSettingsScalabilitySubsystem: Setting scalability quality from benchmark (ForceRun={0})", bForceRunBenchmark);

	// Run benchmark if needed
	if (bForceRunBenchmark || !IsBenchmarkDataAvailable())
	{
		UE_LOGFMT(LogAutoSettings, Log, "AutoSettingsScalabilitySubsystem: Running benchmark before applying results");
		RunBenchmark();
	}

	// Apply cached results
	if (CachedResults.IsSet())
	{
		SetQualityLevels(CachedResults.GetValue());
		UE_LOGFMT(LogAutoSettings, Log, "AutoSettingsScalabilitySubsystem: Set scalability quality from benchmark results");
	}
	else
	{
		UE_LOGFMT(LogAutoSettings, Warning, "AutoSettingsScalabilitySubsystem: No benchmark results available to set quality from");
	}
}

void UAutoSettingsScalabilitySubsystem::SetQualityFromPreset(int32 QualityLevel)
{
	// Clamp to valid range
	QualityLevel = FMath::Clamp(QualityLevel, 0, 4);

	UE_LOGFMT(LogAutoSettings, Log, "AutoSettingsScalabilitySubsystem: Setting all scalability settings to preset quality level {0}", QualityLevel);

	// Create quality levels struct with all settings at the same level
	Scalability::FQualityLevels UniformLevels;
	UniformLevels.SetFromSingleQualityLevel(QualityLevel);

	// Apply to all scalability settings
	SetQualityLevels(UniformLevels);

	UE_LOGFMT(LogAutoSettings, Log, "AutoSettingsScalabilitySubsystem: Successfully applied preset quality level {0} to all scalability settings", QualityLevel);
}

void UAutoSettingsScalabilitySubsystem::SetQualityLevels(const Scalability::FQualityLevels& QualityLevels)
{
	// Update GameUserSettings to keep it in sync
	// GameUserSettings unfortunately does not pick up scalability changes automatically
	if (GEngine)
	{
		if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
		{
			GameUserSettings->ScalabilityQuality = QualityLevels;
		}
	}

	// Apply the quality levels to the scalability system
	// Force update so that settings detect their new defaults when called from containing settings
	Scalability::SetQualityLevels(QualityLevels);
}