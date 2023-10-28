// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityCacheSubsystem.h"
#include "GravityMovementComponent.h"
#include "TimeBasedObject.h"
#include "GravityActor.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Editor/Transactor.h"
#include "TickTaskManagerInterface.h"
#include "Components/SplineComponent.h"
#include "SubobjectDataSubsystem.h"
#include "SubobjectDataHandle.h"

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

	if (!PropertyChangedEvent.Property)
	{
		// Probably not changing an actor
		return;
	}

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

	UPackage* TempPackage = CreatePackage(nullptr);
	TempPackage->MarkAsFullyLoaded();

	UPackage* InPackage = InWorld->GetOutermost();

	FName WorldName = MakeUniqueObjectName(TempPackage, UWorld::StaticClass(), FName(FString::Printf(TEXT("SimWorld"))));
	/*
	FObjectDuplicationParameters Parameters(InWorld, TempPackage);
	Parameters.DestName = InWorld->GetFName();
	Parameters.DestClass = InWorld->GetClass();
	Parameters.DuplicateMode = EDuplicateMode::PIE;
	Parameters.PortFlags = PPF_DuplicateForPIE;

	UWorld* World = CastChecked<UWorld>(StaticDuplicateObjectEx(Parameters));
	UWorld* World = DuplicateObject<UWorld>(InWorld, TempPackage, WorldName);
	World->WorldType = EWorldType::Game;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, "SimWorld", nullptr, false);
	*/
	UWorld* World = NewObject<UWorld>(TempPackage, WorldName);
	World->SetFlags(RF_Transactional);
	World->WorldType = EWorldType::Game;
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	World->PersistentLevel = NewObject<ULevel>(this, TEXT("PersistentLevel"));
	World->PersistentLevel->Initialize(FURL(nullptr));
	World->PersistentLevel->Model = NewObject<UModel>(World->PersistentLevel);
	World->PersistentLevel->Model->Initialize(nullptr, 1);
	World->PersistentLevel->OwningWorld = World;
	World->SetCurrentLevel(World->PersistentLevel);

	World->PersistentLevel->SetWorldSettings(
		World->SpawnActor<AWorldSettings>(GEngine->WorldSettingsClass)
	);

	ULevel* DuplicatedLevel = DuplicateObject<ULevel>(InWorld->PersistentLevel, World, FName("SimLevel"));
	World->PersistentLevel = DuplicatedLevel;
	World->InitWorld(UWorld::InitializationValues()
		.ShouldSimulatePhysics(true)
		.EnableTraceCollision(true)
		.AllowAudioPlayback(false)
		.CreateFXSystem(false));

	struct FSplineConnection {
		USplineComponent* Owner;
		const USplineComponent* Simulating;
		int index = 0;
	};

	FURL URL;
	World->InitializeActorsForPlay(URL);

	if (InWorld->PersistentLevel->Actors.Num() != World->PersistentLevel->Actors.Num())
	{
		bIsRecaching = false;
		return;
	}
	
	TArray<FSplineConnection> Splines;
	for (int i = 0; i < InWorld->PersistentLevel->Actors.Num(); ++i)
	{
		AActor* OriginalActor = InWorld->PersistentLevel->Actors[i];
		if (!OriginalActor)
		{
			continue;
		}

		if (USplineComponent* Spline = OriginalActor->GetComponentByClass<USplineComponent>())
		{
			FSplineConnection Connection;
			Connection.Owner = Spline;
			Connection.Owner->SetSplinePoints({}, ESplineCoordinateSpace::Local, false);
			Connection.Simulating = World->PersistentLevel->Actors[i]->GetComponentByClass<USplineComponent>();
			Connection.index = i;

			if (Connection.Owner && Connection.Simulating)
			{
				Splines.Add(Connection);
			}
		}
	}
	World->BeginPlay();

	for (const auto& Actor : World->PersistentLevel->Actors)
	{
		Actor->DispatchBeginPlay(false);
	}

	int j = 0;
	constexpr static float FixedTimeStep = 1.0f / 60.0f;
	for (float t = 0; t < 60; t += FixedTimeStep)
	{
		if ((j++ % 60) == 0)
		{
			for (FSplineConnection& Spline : Splines)
			{
				if (ensure(Spline.Simulating))
				{
					Spline.Owner->AddSplinePoint(Spline.Simulating->GetOwner()->GetActorLocation(), ESplineCoordinateSpace::Local, false);
				}
			}
		}

		World->Tick(LEVELTICK_All, 1.0f / 60.0f);
		GFrameCounter++;
	}

	FProperty* SplineCurvesProperty = FindFProperty<FProperty>(USplineComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(USplineComponent, SplineCurves));
	for (FSplineConnection& Spline : Splines)
	{
		Spline.Owner->UpdateSpline();
		Spline.Owner->Duration = 60.0f;

		Spline.Owner->bSplineHasBeenEdited = true;

		FComponentVisualizer::NotifyPropertyModified(Spline.Owner, SplineCurvesProperty);
	}

	bIsRecaching = false;

	World->DestroyWorld(false);

	// idk man, shit
	GUnrealEd->Trans->SetUndoBarrier();
}
