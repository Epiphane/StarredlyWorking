// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityActor.h"
#include "GravityComponent.h"

// Sets default values
AGravityActor::AGravityActor()
{
}

void AGravityActor::OnRemove_Implementation()
{
    for (UActorComponent* Component : GetComponents())
    {
        if (UGravityComponent* Gravity = Cast<UGravityComponent>(Component))
        {
            Gravity->OnRemove();
        }
    }
}

void AGravityActor::OnRestore_Implementation()
{
    for (UActorComponent* Component : GetComponents())
    {
        if (UGravityComponent* Gravity = Cast<UGravityComponent>(Component)) 
        {
            Gravity->OnRestore();
        }
    }
}

