// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Ability/AuraGameplayAbility.h"

bool UAuraGameplayAbility::IsAvatarPlayer() const
{
	if (const AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		return Avatar->ActorHasTag(FName("Player"));
	}
	return false;
}