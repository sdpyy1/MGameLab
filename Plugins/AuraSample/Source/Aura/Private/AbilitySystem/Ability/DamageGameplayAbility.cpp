// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/DamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"

void UDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	FGameplayEffectSpecHandle OutgoingGameplayEffectSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass,1.f);
	const float DamageEval = EvalDamage();
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OutgoingGameplayEffectSpec, FAuraGameplayTags::Get().Damage, DamageEval);
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*OutgoingGameplayEffectSpec.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}

float UDamageGameplayAbility::EvalDamage()
{
	return DamageComputer.GetValueAtLevel(GetAbilityLevel());
}
