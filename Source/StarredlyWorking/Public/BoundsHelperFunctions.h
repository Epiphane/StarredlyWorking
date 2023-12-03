// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BoundsHelperFunctions.generated.h"

/**
 * 
 */
UCLASS()
class STARREDLYWORKING_API UBoundsHelperFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, BlueprintCallable)
	static FBoxSphereBounds Combine(const FBoxSphereBounds& A, const FBoxSphereBounds& B);
};
