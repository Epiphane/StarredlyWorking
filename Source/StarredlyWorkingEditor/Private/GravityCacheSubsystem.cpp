// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityCacheSubsystem.h"
#include "GravityMovementComponent.h"
#include "TimeBasedObject.h"

DEFINE_LOG_CATEGORY_STATIC(LogGravityCache, Log, All);

void UGravityCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
#if WITH_EDITORONLY_DATA
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UGravityCacheSubsystem::OnObjectPostEditChange);
#endif
}

bool UGravityCacheSubsystem::IsActorInSimulation(AActor* Actor)
{
	// GravityMovementComponents DEFINITELY affect the simulation.
	if (Actor->GetComponentByClass<UGravityMovementComponent>() != nullptr)
	{
		return true;
	}

	return false;
}

void UGravityCacheSubsystem::OnObjectPostEditChange(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	bool bUpdateCache = false;

	if (Object->Implements<UTimeBasedObject>())
	{
		// TimeBasedObjects probably affect the simulation.
		bUpdateCache = true;
	}
	else if (AActor* Actor = Cast<AActor>(Object))
	{
		if (IsActorInSimulation(Actor))
		{
			bUpdateCache = true;
		}
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(Object))
	{
		if (IsActorInSimulation(Component->GetOwner()))
		{
			bUpdateCache = true;
		}
	}

	if (bUpdateCache)
	{
		RecacheSplines(Object->GetWorld());
	}
}

void UGravityCacheSubsystem::RecacheSplines(UWorld* InWorld)
{
	if (bIsRecaching)
	{
		return;
	}

	bIsRecaching = true;

	//UWorld* World = DuplicateObject<UWorld>(InWorld, GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), "SimulationWorld"));
	//UE_LOG(LogGravityCache, Log, TEXT("Planet pos: %s"))
	UWorld* World = UWorld::CreateWorld(EWorldType::GamePreview, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::GamePreview);
	WorldContext.SetCurrentWorld(World);

	FURL URL;
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	World->Tick(ELevelTick::LEVELTICK_All, 1.0f / 60.0f);

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	//World->DestroyWorld();

	bIsRecaching = false;
}
