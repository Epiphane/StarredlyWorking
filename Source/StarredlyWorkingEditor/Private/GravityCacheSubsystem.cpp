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
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogGravityCache, Log, All);

void UGravityCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
#if WITH_EDITORONLY_DATA
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UGravityCacheSubsystem::OnObjectPostEditChange);
#endif
}

void UGravityCacheSubsystem::Tick(float DeltaTime)
{
	if (bIsDirty)
	{
		//GEditor->GetSelectionStateOfLevel
		RecacheSplines(GWorld);
		bIsDirty = false;
	}
}

bool UGravityCacheSubsystem::IsActorInSimulation(AActor* Actor)
{
	// GravityMovementComponents DEFINITELY affect the simulation.
	if (Actor && Actor->GetComponentByClass<UGravityMovementComponent>() != nullptr)
	{
		return true;
	}

	return false;
}

void UGravityCacheSubsystem::OnObjectPostEditChange(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	bool bUpdateCache = false;

	if (!Object || !PropertyChangedEvent.Property)
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
		bIsDirty = true;

	}
}

void UGravityCacheSubsystem::RecacheSplines(UWorld* InWorld)
{
	if (bIsRecaching)
	{
		return;
	}

	bIsRecaching = true;

	UPackage* InPackage = InWorld->GetOutermost();

	UPackage* SimContainer = CreatePackage(TEXT("/Temp/SimContainer"));
	SimContainer->MarkAsFullyLoaded();

	FName WorldName = MakeUniqueObjectName(SimContainer, UWorld::StaticClass(), FName(FString::Printf(TEXT("SimWorld"))));
	UWorld* World = NewObject<UWorld>(SimContainer, WorldName);
	World->SetFlags(RF_Transactional);
	World->WorldType = EWorldType::Game;
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	World->InitializeNewWorld({}, true);
	
	// Duplicate the level to our own and then initialize it.
	World->PersistentLevel = DuplicateObject<ULevel>(InWorld->PersistentLevel, World, FName("SimLevel"));
	World->InitWorld(UWorld::InitializationValues()
		.ShouldSimulatePhysics(true)
		.EnableTraceCollision(true)
		.AllowAudioPlayback(false)
		.CreateFXSystem(false));

	struct FSplineConnection {
		USplineComponent* Owner;
		const USplineComponent* Simulating;
		FVector Initial;
	};

	FURL URL;
	World->InitializeActorsForPlay(URL);

	TArray<FSplineConnection> Splines;
	for (TActorIterator<AActor> It(InWorld); It; ++It)
	{
		FName Search = It->GetFName();
		TObjectPtr<AActor>* Other = World->PersistentLevel->Actors.FindByPredicate([&](const AActor* Actor) { return Actor->GetFName() == Search; });
		
		if (!Other)
		{
			continue;
		}

		if (USplineComponent* Spline = It->GetComponentByClass<USplineComponent>())
		{
			FSplineConnection Connection;
			Connection.Owner = Spline;
			Connection.Owner->SetSplinePoints({}, ESplineCoordinateSpace::Local, false);
			Connection.Simulating = (*Other)->GetComponentByClass<USplineComponent>();
			Connection.Initial = Spline->GetOwner()->GetActorLocation();

			if (Connection.Owner && Connection.Simulating)
			{
				Splines.Add(Connection);
			}
		}
	}

	World->BeginPlay();
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		It->DispatchBeginPlay(false);
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
					Spline.Owner->AddSplinePoint(Spline.Simulating->GetOwner()->GetActorLocation() - Spline.Initial, ESplineCoordinateSpace::Local, false);
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
	
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		It->Destroy();
	}
	World->DestroyWorld(false);
	World->PersistentLevel->MarkPendingKill();
	World->PersistentLevel = nullptr;
	World->MarkPendingKill();
	World->RemoveFromRoot();
}
