// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Ability/AuraMeleeAttack.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

bool UAuraMeleeAttack::IsPlayerWithinNotifyRadius(float Radius) const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return false;
	}

	// 直接以命中帧的 Socket 世界坐标作为范围中心
	const FVector NotifyLocation = GetNotifyLocation();

	// 在 NotifyLocation 的 Radius 球形范围内查询所有存活角色
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Avatar); // 排除自己
	UAuraAbilitySystemLibrary::GetLivePlayerWithinRadius(Avatar, OverlappingActors, ActorsToIgnore, NotifyLocation, Radius);

	// 遍历结果，用 "Player" Tag 判断是否存在玩家
	for (const AActor* Actor : OverlappingActors)
	{
		if (IsValid(Actor) && Actor->ActorHasTag(FName("Player")))
		{
			return true;
		}
	}
	return false;
}
