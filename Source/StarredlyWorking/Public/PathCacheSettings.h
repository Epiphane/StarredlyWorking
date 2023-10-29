// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PathCacheSettings.generated.h"

// Defines the settings used when caching planet paths.
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Path Cache Settings"))
class STARREDLYWORKING_API UPathCacheSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Turn up for higher fidelity, turn down for higher performance.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Path Caching", meta = (ClampMin = "1", ClampMax = "60", UIMin = "1", UIMax = "60"))
	int SimulateFPS = 60;

	// Number of simulation seconds to tick per engine-frame. Turn down for slower caching but higher general performance.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Path Caching", meta = (ClampMin = "1", ClampMax = "60", UIMin = "1", UIMax = "60"))
	int SimulationSpeed = 30;

	// How long the simulation should be.
	// TODO maybe this should be a level/world-specific setting.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Path Caching", meta = (ClampMin = "1", ClampMax = "360", UIMin = "1", UIMax = "360"))
	int SimulateTime = 60;

};
