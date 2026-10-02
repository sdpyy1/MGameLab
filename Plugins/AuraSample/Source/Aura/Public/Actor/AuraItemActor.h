// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraItemActor.generated.h"

UCLASS()
class AURA_API AAuraItemActor : public AActor
{
	GENERATED_BODY()

public:
	AAuraItemActor();

protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Floating")
	float FloatAmplitude = 20.f;

	UPROPERTY(EditAnywhere, Category = "Floating")
	float FloatSpeed = 2.f;

	FVector StartLocation;
	float RunningTime = 0.f;
};
