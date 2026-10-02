// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraAttackGameplayAbility.h"
#include "AbilitySystem/Ability/AuraGameplayAbility.h"
#include "DamageGameplayAbility.generated.h"

/**
 *	会造成伤害的Ability基类
 */
UCLASS()
class AURA_API UDamageGameplayAbility : public UAuraAttackGameplayAbility
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor * TargetActor);

	UFUNCTION(BlueprintCallable)
	float EvalDamage();

	/*
		GameplayEffect类型，用于施加伤害
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/*
	 *	伤害计算器
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FScalableFloat DamageComputer;
};
