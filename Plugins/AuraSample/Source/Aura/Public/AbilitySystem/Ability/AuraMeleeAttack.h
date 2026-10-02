// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageGameplayAbility.h"
#include "AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AuraMeleeAttack.generated.h"

/**
 *	近战攻击能力
 */
UCLASS()
class AURA_API UAuraMeleeAttack : public UDamageGameplayAbility
{
	GENERATED_BODY()

public:
	// 判断在以命中 Socket（GetNotifyLocation）为中心的 Radius 范围内是否存在带 "Player" Tag 的存活角色（已排除自身）
	UFUNCTION(BlueprintPure, Category = "Montage")
	bool IsPlayerWithinNotifyRadius(float Radius = 200.f) const;
};
