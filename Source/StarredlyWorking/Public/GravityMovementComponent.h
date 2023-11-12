// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GravityComponent.h"
#include "GravityMovementComponent.generated.h"

UCLASS(meta=(BlueprintSpawnableComponent))
class STARREDLYWORKING_API UGravityMovementComponent : public UGravityComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGravityMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnRemove_Implementation() override;
	virtual void OnRestore_Implementation() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Density = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanPullObjects = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBePulled = true;
};
