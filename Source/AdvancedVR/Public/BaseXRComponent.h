// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseXRComponent.generated.h"


UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class ADVANCEDVR_API UBaseXRComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBaseXRComponent();
		
};
