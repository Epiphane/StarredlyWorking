// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "GravityCacheSubsystem.generated.h"

UCLASS()
class STARREDLYWORKINGEDITOR_API UGravityCacheSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	static bool IsActorInSimulation(AActor* Actor);
	void OnObjectPostEditChange(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

	void RecacheSplines(UWorld* World);
	bool bIsRecaching = false;
};
