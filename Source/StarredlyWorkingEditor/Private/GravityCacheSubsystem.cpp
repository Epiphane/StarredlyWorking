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
#include "SimulationCacheComponent.h"
#include "EngineUtils.h"
#include "PathCacheSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogGravityCache, Log, All);

void UGravityCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
#if WITH_EDITORONLY_DATA
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UGravityCacheSubsystem::OnObjectPostEditChange);
#endif

	SplineCurvesProperty = FindFProperty<FProperty>(USplineComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(USplineComponent, SplineCurves));
}

void UGravityCacheSubsystem::Tick(float DeltaTime)
{
	if (bIsDirty)
	{
		if (!SimWorld)
		{
			CreateSimulation(GWorld);
			bIsDirty = false;
		}
	}

	const UPathCacheSettings* CacheSettings = GetDefault<UPathCacheSettings>();
	if (SimWorld)
	{
		for (int i = 0; i < CacheSettings->SimulationSpeed * CacheSettings->SimulateFPS; ++i)
		{
			if (SimWorld->TimeSeconds < CacheSettings->SimulateTime)
			{
				TickWorldOneTime(1.0f / CacheSettings->SimulateFPS);
			}
			else
			{
				CleanupSimulation();
				return;
			}
		}
	}
}

void UGravityCacheSubsystem::CreateSimulation(UWorld* Base)
{
	UPackage* SimContainer = CreatePackage(TEXT("/Temp/SimContainer"));
	SimContainer->MarkAsFullyLoaded();
	FName WorldName = MakeUniqueObjectName(SimContainer, UWorld::StaticClass(), FName(FString::Printf(TEXT("SimWorld"))));
	SimWorld = NewObject<UWorld>(SimContainer, WorldName);
	SimWorld->SetFlags(RF_Transactional);
	SimWorld->WorldType = EWorldType::Game;
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(SimWorld);

	SimWorld->InitializeNewWorld({}, true);

	// Duplicate the level to our own and then initialize it.
	SimWorld->PersistentLevel = DuplicateObject<ULevel>(Base->PersistentLevel, SimWorld, FName("SimLevel"));
	SimWorld->InitWorld(UWorld::InitializationValues()
		.ShouldSimulatePhysics(true)
		.EnableTraceCollision(true)
		.AllowAudioPlayback(false)
		.CreateFXSystem(false));

	FURL URL;
	SimWorld->InitializeActorsForPlay(URL);

	// Create the SimSplines list, which is a mapping from simulation actor to real level actor.
	SimSplines.Empty();
	for (TActorIterator<AActor> It(Base); It; ++It)
	{
		FName Search = It->GetFName();
		TObjectPtr<AActor>* Other = SimWorld->PersistentLevel->Actors.FindByPredicate([&](const AActor* Actor) { return Actor->GetFName() == Search; });

		if (!Other)
		{
			continue;
		}

		if (USplineComponent* Spline = It->GetComponentByClass<USplineComponent>())
		{
			FSplineConnection Connection;
			Connection.Owner = Spline;
			Connection.Owner->SetSplinePoints({}, ESplineCoordinateSpace::Local, false);
			Connection.Simulating = *Other;
			Connection.InitialPosition = Spline->GetOwner()->GetActorLocation();
			Connection.InitialRotation = Spline->GetOwner()->GetActorRotation();

			if (Connection.Owner && Connection.Simulating)
			{
				SimSplines.Add(Connection);
			}
		}

		if (USimulationCacheComponent* Cache = It->GetComponentByClass<USimulationCacheComponent>())
		{
			Cache->Position.Points.Empty();
			Cache->Velocity.Points.Empty();;
			Cache->Rotation.Points.Empty();;
			Cache->AngularVelocity.Points.Empty();;
		}
	}

	// Call BeginPlay so things will simulate.
	SimWorld->BeginPlay();
	for (TActorIterator<AActor> It(SimWorld); It; ++It)
	{
		It->DispatchBeginPlay(false);
	}

	LastCachedSecond = -1;
}

void UGravityCacheSubsystem::TickWorldOneTime(float TimeStep)
{
	if (!ensure(TimeStep > 0))
	{
		return;
	}

	SimWorld->Tick(LEVELTICK_All, TimeStep);

	int CurrentSecond = FMath::TruncToInt(SimWorld->TimeSeconds);
	if (CurrentSecond > LastCachedSecond)
	{
		for (FSplineConnection& Spline : SimSplines)
		{
			if (ensure(Spline.Simulating))
			{
				FSplinePoint Point;
				FVector Velocity = Spline.Simulating->GetVelocity();
				Point.LeaveTangent = Velocity;
				Point.ArriveTangent = -Velocity;
				Point.InputKey = SimWorld->TimeSeconds;
				Point.Position = (Spline.Simulating->GetActorLocation() - Spline.InitialPosition) / Spline.Simulating->GetActorScale();
				Point.Rotation = Spline.Simulating->GetActorRotation();
				Point.Type = ESplinePointType::Type::Curve;
				Spline.Owner->AddPoint(Point);
				
				USimulationCacheComponent* Cache = Spline.Owner->GetOwner()->GetComponentByClass<USimulationCacheComponent>();
				UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Spline.Simulating->GetRootComponent());
				if (Cache && Primitive)
				{
					Cache->Position.Points.Add({ (float)SimWorld->TimeSeconds, Spline.Simulating->GetActorLocation(), {}, {}, CIM_CurveAuto });
					Cache->Velocity.Points.Add({ (float)SimWorld->TimeSeconds, Velocity, {}, {}, CIM_CurveAuto });
					Cache->Rotation.Points.Add({ (float)SimWorld->TimeSeconds, Point.Rotation.Quaternion(), {}, {}, CIM_CurveAuto });
					Cache->AngularVelocity.Points.Add({ (float)SimWorld->TimeSeconds, Primitive->GetPhysicsAngularVelocityInDegrees(), {}, {}, CIM_CurveAuto });
				}
			}
		}

		LastCachedSecond = CurrentSecond;
	}

	GFrameCounter++;
}

void UGravityCacheSubsystem::CleanupSimulation()
{
	if (!ensure(IsValid(SimWorld)))
	{
		return;
	}

	// Persist spline data to the actual actors.
	for (FSplineConnection& Spline : SimSplines)
	{
		Spline.Owner->UpdateSpline();
		Spline.Owner->Duration = SimWorld->TimeSeconds;
		Spline.Owner->bSplineHasBeenEdited = true;

		FComponentVisualizer::NotifyPropertyModified(Spline.Owner, SplineCurvesProperty);

		if (USimulationCacheComponent* Cache = Spline.Owner->GetOwner()->GetComponentByClass<USimulationCacheComponent>())
		{
			Cache->Position.AutoSetTangents(0.0f, false);
			Cache->Velocity.AutoSetTangents(0.0f, false);
			Cache->Rotation.AutoSetTangents(0.0f, false);
			Cache->AngularVelocity.AutoSetTangents(0.0f, false);
		}
	}

	for (TActorIterator<AActor> It(SimWorld); It; ++It)
	{
		It->Destroy();
	}
	SimWorld->DestroyWorld(false);
	SimWorld->PersistentLevel->MarkAsGarbage();
	SimWorld->PersistentLevel = nullptr;
	SimWorld->MarkAsGarbage();
	SimWorld->RemoveFromRoot();
	GEngine->DestroyWorldContext(SimWorld);
	SimWorld->GetPackage()->ClearDirtyFlag();
	SimWorld = nullptr;
	SimSplines.Empty();
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

	if (!Object || 
		!PropertyChangedEvent.Property ||
		PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive ||
		PropertyChangedEvent.Property == SplineCurvesProperty)
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
}
