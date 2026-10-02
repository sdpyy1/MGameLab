// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraItemActor.h"
#include "GameplayEffectTypes.h"

#include "AuraEffectActor.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

UCLASS()
class AURA_API AAuraEffectActor : public AAuraItemActor
{
	GENERATED_BODY()

public:
	AAuraEffectActor();

protected:
	virtual void BeginPlay() override;

	/*
		给一个Actor施加一个GameplayEffect
	*/
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	UFUNCTION(BlueprintCallable)
	void RemoveEffectFromTarget(AActor* TargetActor);

	//////////////////////  Instance Effect  //////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> InstanceGameplayEffectClass;

	//////////////////////  Infinite Effect  //////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;

	TMap<UAbilitySystemComponent*, FActiveGameplayEffectHandle> ActiveEffectMap;
};
