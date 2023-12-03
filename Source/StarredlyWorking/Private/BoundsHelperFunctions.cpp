// Fill out your copyright notice in the Description page of Project Settings.


#include "BoundsHelperFunctions.h"

FBoxSphereBounds UBoundsHelperFunctions::Combine(const FBoxSphereBounds& A, const FBoxSphereBounds& B)
{
    return A + B;
}
