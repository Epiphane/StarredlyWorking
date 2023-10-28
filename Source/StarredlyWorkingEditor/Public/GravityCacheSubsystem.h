// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "GravityCacheSubsystem.generated.h"

UCLASS()
class STARREDLYWORKINGEDITOR_API UGravityCacheSubsystem : public UEditorSubsystem, public FTickableEditorObject
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override { return TStatId(); }

private:
	static bool IsActorInSimulation(AActor* Actor);
	void OnObjectPostEditChange(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

	void RecacheSplines(UWorld* World);
	bool bIsRecaching = false;
	bool bIsDirty = false;
};
