// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TimeBasedObject.generated.h"

UINTERFACE(Blueprintable, Category = "Time")
class STARREDLYWORKING_API UTimeBasedObject : public UInterface
{
	GENERATED_BODY()
	
};

class ITimeBasedObject
{
    GENERATED_BODY()

public:
    /** Add interface function declarations here */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void SetTime(float CurrentTime);
};
