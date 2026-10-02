// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "AuraGameplayAbility.generated.h"

/**
 *	GameAbility
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	
	// 赋予能力时放入DynamicAbilityTags
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag InputMappingTag;

	// 判断本能力的持有者（Avatar）是否为玩家：项目约定玩家带 "Player" Tag，否则视为敌人
	UFUNCTION(BlueprintPure, Category = "Aura|Utility")
	bool IsAvatarPlayer() const;


};
