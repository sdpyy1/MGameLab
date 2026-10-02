// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/AuraItemActor.h"

AAuraItemActor::AAuraItemActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAuraItemActor::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();
}

void AAuraItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	const float OffsetZ = FMath::Sin(RunningTime * FloatSpeed) * FloatAmplitude;

	FVector NewLocation = StartLocation;
	NewLocation.Z += OffsetZ;

	SetActorLocation(NewLocation);
}