// Fill out your copyright notice in the Description page of Project Settings.


#include "SimulationCacheComponent.h"

// Sets default values for this component's properties
USimulationCacheComponent::USimulationCacheComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USimulationCacheComponent::GetStateAtTime(float Time, FSimulationCacheState& OutState) const
{
	OutState.Time = Time;
	OutState.Position = Position.Eval(Time, FVector::ZeroVector);
	OutState.Velocity = Velocity.Eval(Time, FVector::ZeroVector);
	OutState.Rotation = Rotation.Eval(Time, FQuat::Identity);
	OutState.AngularVelocity = AngularVelocity.Eval(Time, FVector::ZeroVector);
}

