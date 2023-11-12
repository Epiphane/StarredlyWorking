// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityMovementComponent.h"
#include "GravitySubsystem.h"

UGravityMovementComponent::UGravityMovementComponent()
{}

void UGravityMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGravitySubsystem* Gravity = GetWorld() ? GetWorld()->GetSubsystem<UGravitySubsystem>() : nullptr)
	{
		Gravity->TrackGravitySource(this);
	}
}

void UGravityMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGravitySubsystem* Gravity = GetWorld() ? GetWorld()->GetSubsystem<UGravitySubsystem>() : nullptr)
	{
		Gravity->RemoveGravitySource(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UGravityMovementComponent::OnRemove_Implementation()
{
	if (UGravitySubsystem* Gravity = GetWorld() ? GetWorld()->GetSubsystem<UGravitySubsystem>() : nullptr)
	{
		Gravity->RemoveGravitySource(this);
	}
}

void UGravityMovementComponent::OnRestore_Implementation()
{
	if (UGravitySubsystem* Gravity = GetWorld() ? GetWorld()->GetSubsystem<UGravitySubsystem>() : nullptr)
	{
		Gravity->TrackGravitySource(this);
	}
}

