// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/AuraEffectActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include <AbilitySystemInterface.h>
#include "AbilitySystemBlueprintLibrary.h"
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include "GameplayTagsManager.h"
AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;
	check(GameplayEffectClass);
	FGameplayEffectContextHandle effectContextHandle = TargetASC->MakeEffectContext();
	effectContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle effectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, 1.f, effectContextHandle);
	FActiveGameplayEffectHandle GameplayEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*effectSpecHandle.Data.Get());

	const bool bIsInfinite = effectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite) {
		ActiveEffectMap.Add(TargetASC, GameplayEffectHandle);
	}
}

void AAuraEffectActor::RemoveEffectFromTarget(AActor* TargetActor)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (ActiveEffectMap.Contains(TargetASC)) {
		TargetASC->RemoveActiveGameplayEffect(ActiveEffectMap[TargetASC]);
	}
	ActiveEffectMap.Remove(TargetASC);
}