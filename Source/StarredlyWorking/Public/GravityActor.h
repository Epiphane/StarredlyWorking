// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GravityActor.generated.h"

UCLASS(BlueprintType)
class STARREDLYWORKING_API AGravityActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGravityActor();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnRemove();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnRestore();
};
