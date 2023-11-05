// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GravitySettings.generated.h"

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Gravity Settings"))
class STARREDLYWORKING_API UGravitySettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// Defines the exponent used on distance. Turn up in order to weight distance less heavily
	// when calculating gravity.
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Gravity")
	float DistanceStrength = 2;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Gravity")
	float G = 9.81f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Gravity")
	bool bShowDebugArrows = false;
};
