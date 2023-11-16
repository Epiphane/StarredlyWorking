// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityResetSubsystem.h"

#include "EngineUtils.h"
#include "GravityActor.h"
#include "GravityComponent.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogGravityResetSubsystem, Log, All);

static int32 SortComponents(UActorComponent* A, UActorComponent* B)
{
    return A->GetName().Compare(B->GetName());
}

void UGravityResetSubsystem::SaveCurrentState()
{
    UWorld* World = GetWorld();
    TArray<AGravityActor*> ActorsToDuplicate;
    for (TActorIterator<AGravityActor> It(World); It; ++It)
    {
        ActorsToDuplicate.Add(*It);
    }

    ActorsToDuplicate.Sort([](const AGravityActor& A, const AGravityActor& B) {
        int ADepth = 0;
        for (AActor* Parent = A.GetAttachParentActor(); Parent != nullptr; Parent = Parent->GetAttachParentActor())
        {
            ++ADepth;
        }

        int BDepth = 0;
        for (AActor* Parent = B.GetAttachParentActor(); Parent != nullptr; Parent = Parent->GetAttachParentActor())
        {
            ++BDepth;
        }

        return BDepth > ADepth;
    });

    TMap<AActor*, AActor*> BaseToCopy;

    for (AGravityActor* Base : ActorsToDuplicate)
    {
        FActorSpawnParameters Params;
        Params.Name = Base->GetFName();
        Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
        Params.Template = Base;
        Params.Owner = Base->GetOwner();
        Params.OverrideLevel = Base->GetLevel();
        AGravityActor* Transient = World->SpawnActor<AGravityActor>(Base->GetClass(), Params);
        BaseToCopy.Emplace(Base, Transient);

        if (AActor* Parent = Base->GetAttachParentActor())
        {
            FName Socket = Base->GetAttachParentSocketName();
            Transient->AttachToActor(BaseToCopy.FindChecked(Parent), FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true), Socket);
        }

        // Duplicate all components separately.
        TArray<UActorComponent*> BaseComponents = Base->GetComponents().Array();
        TArray<UActorComponent*> TransientComponents = Transient->GetComponents().Array();
        const auto Predicate = [](UActorComponent*& A, UActorComponent*& B) { return A->GetName().Compare(B->GetName()); };
        BaseComponents.Sort();// &SortComponents);
        TransientComponents.Sort();// &SortComponents);

        if (!ensure(BaseComponents.Num() == TransientComponents.Num()))
        {
            UE_LOG(LogGravityResetSubsystem, Error, TEXT("Component Length got changed! Skipping %s"), *Base->GetName());
            continue;
        }

        auto BaseIt = BaseComponents.begin();
        auto TransIt = TransientComponents.begin();
        for (; BaseIt != BaseComponents.end(); ++BaseIt, ++TransIt)
        {
            UActorComponent* BaseComp = *BaseIt;
            UActorComponent* TransComp = *TransIt;
            if (!ensure(BaseComp->GetClass() == TransComp->GetClass()))
            {
                UE_LOG(LogGravityResetSubsystem, Error, TEXT("Components got out of order! Aborting %s"), *Base->GetName());
                break;
            }

            if (!BaseComp->IsA<UGravityComponent>())
            {
                continue;
            }

            CopyComponent(BaseComp, TransComp);
        }

        BackupObjects.Add(Base);
        Remove(Base);
    }
}

void UGravityResetSubsystem::RestoreState()
{
    UWorld* World = GetWorld();
    for (TActorIterator<AGravityActor> It(World); It; ++It)
    {
        if (!BackupObjects.Contains(*It))
        {
            Remove(*It);
            World->DestroyActor(*It);
        }
    }

    for (AGravityActor* Original : BackupObjects)
    {
        Restore(Original);
    }
    BackupObjects.Empty();
}

void UGravityResetSubsystem::Remove(AGravityActor* Actor, bool bSkipNotify)
{
    Actor->OnRemove();

    Actor->SetActorHiddenInGame(true);
    Actor->SetActorTickEnabled(false);
    for (UActorComponent* Component : Actor->GetComponents())
    {
        if (UGravityComponent* GravComp = Cast<UGravityComponent>(Component))
        {
            GravComp->OnRemove();
        }

        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component))
        {
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        Component->SetComponentTickEnabled(false);
    }
}

void UGravityResetSubsystem::Restore(AGravityActor* Actor)
{
    Actor->SetActorHiddenInGame(false);
    Actor->SetActorTickEnabled(true);
    for (UActorComponent* Component : Actor->GetComponents())
    {
        Component->Activate();
        if (UGravityComponent* GravComp = Cast<UGravityComponent>(Component))
        {
            GravComp->OnRestore();
        }

        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component))
        {
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        Component->SetComponentTickEnabled(true);
    }

    Actor->OnRestore();
}

void UGravityResetSubsystem::CopyComponent(UActorComponent* From, UActorComponent* To)
{
    // TODO: This should be closer to DuplicateObject, instead I'm going with a hella naive approach that just
    // tries to copy each property individually. It won't duplicate subobjects or other owned classes.
    for (TFieldIterator<FProperty> PropIt(From->GetClass(), EFieldIterationFlags::Default); PropIt; ++PropIt)
    {
        if (!PropIt.GetStruct()->IsChildOf<UGravityComponent>())
        {
            break;
        }
        CopyProperty(*PropIt, From, To);
    }
}

void UGravityResetSubsystem::CopyProperty(FProperty* Property, UActorComponent* From, UActorComponent* To)
{
    if (Property->GetName() == TEXT("UberGraphFrame"))
    {
        return;
    }

    if (const FObjectProperty* ObjectProperty = CastField<const FObjectProperty>(Property))
    {
        UObject* Object = ObjectProperty->GetPropertyValue_InContainer(From);
        if (IsValid(Object) && Object->IsA<UUserWidget>())
        {
            // Don't copy widgets.
            return;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Copying %s.%s"), *From->GetName(), *Property->GetName());
    if (const FIntProperty* IntProperty = CastField<const FIntProperty>(Property))
    {
        IntProperty->SetPropertyValue_InContainer(To, IntProperty->GetPropertyValue_InContainer(From));
    }
    else if (const FFloatProperty* FloatProperty = CastField<const FFloatProperty>(Property))
    {
        FloatProperty->SetPropertyValue_InContainer(To, FloatProperty->GetPropertyValue_InContainer(From));
    }
    else if (const FDoubleProperty* DoubleProperty = CastField<const FDoubleProperty>(Property))
    {
        DoubleProperty->SetPropertyValue_InContainer(To, DoubleProperty->GetPropertyValue_InContainer(From));
    }
    else if (const FBoolProperty* BoolProperty = CastField<const FBoolProperty>(Property))
    {
        BoolProperty->SetPropertyValue_InContainer(To, BoolProperty->GetPropertyValue_InContainer(From));
    }
    else if (const FStrProperty* StrProperty = CastField<const FStrProperty>(Property))
    {
        StrProperty->SetPropertyValue_InContainer(To, StrProperty->GetPropertyValue_InContainer(From));
    }
    else
    {
        UE_LOG(LogGravityResetSubsystem, Error, TEXT("Can't duplicate property: '%s'. Ask Thomas to fix this"), *Property->GetName());
    }
}
