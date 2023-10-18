// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityMovementComponent.h"
#include "GravitySubsystem.h"

// Sets default values for this component's properties
UGravityMovementComponent::UGravityMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGravityMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetSubsystem<UGravitySubsystem>()->TrackGravitySource(this);
}

void UGravityMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetSubsystem<UGravitySubsystem>()->RemoveGravitySource(this);

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UGravityMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

