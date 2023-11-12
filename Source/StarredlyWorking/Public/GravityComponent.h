// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravityComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class STARREDLYWORKING_API UGravityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnRemove();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnRestore();
};
