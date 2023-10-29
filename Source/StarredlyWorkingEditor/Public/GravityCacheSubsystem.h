// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "GravityCacheSubsystem.generated.h"

class USplineComponent;

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
	bool bIsDirty = false;

private:
	void CreateSimulation(UWorld* Base);
	void TickWorldOneTime(float TimeStep);
	void CleanupSimulation();

	struct FSplineConnection {
		USplineComponent* Owner;
		const USplineComponent* Simulating;
		FVector InitialPosition;
	};

	FProperty* SplineCurvesProperty = nullptr;

	void RecacheSplines(UWorld* World);
	UWorld* SimWorld = nullptr;
	TArray<FSplineConnection> SimSplines;
	int LastCachedSecond = 0;
};
