// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GravityResetSubsystem.generated.h"

class AGravityActor;

UCLASS()
class STARREDLYWORKING_API UGravityResetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SaveCurrentState();

	UFUNCTION(BlueprintCallable)
	void RestoreState();

private:
	void Remove(AGravityActor* Actor);
	void Restore(AGravityActor* Actor);
	void CopyComponent(UActorComponent* From, UActorComponent* To);
	void CopyProperty(FProperty* Prop, UActorComponent* From, UActorComponent* To);

	TArray<AGravityActor*> BackupObjects;
};
