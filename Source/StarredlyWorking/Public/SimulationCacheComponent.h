// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimulationCacheComponent.generated.h"

USTRUCT(BlueprintType)
struct FSimulationCacheState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Time;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Position;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FQuat Rotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector AngularVelocity;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STARREDLYWORKING_API USimulationCacheComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USimulationCacheComponent();

	UFUNCTION(BlueprintPure, BlueprintCallable)
	void GetStateAtTime(float Time, FSimulationCacheState& OutState) const;

	UPROPERTY(EditAnywhere, Category = Points)
	FInterpCurveVector Position;

	UPROPERTY(EditAnywhere, Category = Points)
	FInterpCurveVector Velocity;

	UPROPERTY(EditAnywhere, Category = Points)
	FInterpCurveQuat Rotation;

	UPROPERTY(EditAnywhere, Category = Points)
	FInterpCurveVector AngularVelocity;
};
