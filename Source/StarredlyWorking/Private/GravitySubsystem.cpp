// Fill out your copyright notice in the Description page of Project Settings.

#include "GravitySubsystem.h"
#include "DrawDebugHelpers.h"
#include "GravityMovementComponent.h"

void UGravitySubsystem::Tick(float DeltaTime)
{
    bool bNeedsCompact = false;
    for (const auto Comp : GravitySources)
    {
        if (!Comp.IsValid())
        {
            bNeedsCompact = true;
            break;
        }
    }

    if (bNeedsCompact)
    {
        CompactSources();
    }

    for (auto ItA = GravitySources.begin(); ItA != GravitySources.end(); ++ItA)
    {
        UGravityMovementComponent* CompA = (*ItA).Get();

        auto ItB = ItA;
        for (++ItB; ItB != GravitySources.end(); ++ItB)
        {
            UGravityMovementComponent* CompB = (*ItB).Get();

            AActor* ActorA = CompA->GetOwner();
            FVector LocationA = ActorA->GetActorLocation();
            UPrimitiveComponent* BodyA = ActorA->GetComponentByClass<UPrimitiveComponent>();

            AActor* ActorB = CompB->GetOwner();
            FVector LocationB = ActorB->GetActorLocation();
            UPrimitiveComponent* BodyB = ActorB->GetComponentByClass<UPrimitiveComponent>();

            FVector Direction = LocationB - LocationA;
            double Distance = Direction.Length();

            constexpr float G = 5000.0f;
            float Force = G * BodyA->GetMass() * BodyB->GetMass() / Direction.SquaredLength();

            Direction.Normalize();
            double LineDist = FMath::Min(Distance / 2 - 10, 300);
            if (CompA->bCanBePulled && CompB->bCanPullObjects)
            {
                DrawDebugDirectionalArrow(GetWorld(), LocationA, LocationA + LineDist * Direction, 100.0f, FColor::Purple, false, -1.0f, 2, Force / 1000.0f);
                FVector Pull = Force * Direction;
                BodyA->AddForce(Pull);
            }

            if (CompB->bCanBePulled && CompA->bCanPullObjects)
            {
                DrawDebugDirectionalArrow(GetWorld(), LocationB, LocationB + LineDist * -Direction, 100.0f, FColor::Orange, false, -1.0f, 1, Force / 1000.0f);
                FVector Pull = -Force * Direction;
                BodyB->AddForce(Pull);
            }
        }
    }

}

void UGravitySubsystem::TrackGravitySource(UGravityMovementComponent* Source)
{
    GravitySources.Add(TWeakObjectPtr<UGravityMovementComponent>(Source));
}

void UGravitySubsystem::RemoveGravitySource(UGravityMovementComponent* Source)
{
    GravitySources.Remove(TWeakObjectPtr<UGravityMovementComponent>(Source));
}

void UGravitySubsystem::CompactSources()
{
    GravitySources.RemoveAllSwap([](auto& Ptr) { return !Ptr.IsValid(); });
}
