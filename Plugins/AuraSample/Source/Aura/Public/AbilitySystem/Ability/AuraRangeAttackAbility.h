// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageGameplayAbility.h"
#include "AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AuraRangeAttackAbility.generated.h"
class AAuraProjectileActor;

class AActor;
UCLASS()
class AURA_API UAuraRangeAttackAbility : public UDamageGameplayAbility
{
	GENERATED_BODY()
protected:
	
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FVector& SpawnLocation, const FVector& TargetLocation);

	
	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TSubclassOf<AAuraProjectileActor> ProjectileActorClass;
};
