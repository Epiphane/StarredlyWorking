// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GravitySubsystem.generated.h"

class UGravityMovementComponent;

UCLASS()
class STARREDLYWORKING_API UGravitySubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
	// UTickableWorldSubsystem implementation
	virtual void Tick(float DeltaTime) override;
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override { return TStatId(); }

public:
	void TrackGravitySource(UGravityMovementComponent* Source);
	void RemoveGravitySource(UGravityMovementComponent* Source);

private:
	void CompactSources();

	TArray<TWeakObjectPtr<UGravityMovementComponent>> GravitySources;
};
