// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Ability/AuraAttackGameplayAbility.h"
#include "AbilitySystem/AbilityTasks/AbilityTask_PlayMontageAndWaitForEvent.h"
#include "AbilitySystemLog.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/EnemyInterface.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

FVector UAuraAttackGameplayAbility::GetTargetLocation() const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return FVector::ZeroVector;
	}

	// 优先从接口取战斗目标（角色需实现 EnemyInterface）；主角若也设置了该目标则同样走接口
	if (Avatar->Implements<UEnemyInterface>())
	{
		if (AActor* CombatTarget = IEnemyInterface::Execute_GetCombatTarget(Avatar))
		{
			return CombatTarget->GetActorLocation();
		}
	}

	// 接口无有效目标时（如玩家未锁定敌人），退化到鼠标在场景中的落点
	if (IsAvatarPlayer())
	{
		return UAuraAbilitySystemLibrary::GetMouseTargetLocation(Avatar);
	}

	return FVector::ZeroVector;
}

AActor* UAuraAttackGameplayAbility::GetCombatTarget() const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (Avatar->Implements<UEnemyInterface>())
		{
			return IEnemyInterface::Execute_GetCombatTarget(Avatar);
		}
	}
	return nullptr;
}

FTaggedMontage UAuraAttackGameplayAbility::GetRandomAttackMontage() const
{
	FTaggedMontage EmptyMontage;
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return EmptyMontage;
	}

	const TArray<FTaggedMontage> Montages = ICombatInterface::Execute_GetAttackMontages(Avatar);
	if (Montages.Num() == 0)
	{
		return EmptyMontage;
	}

	const int32 Index = FMath::RandRange(0, Montages.Num() - 1);
	return Montages[Index];
}

void UAuraAttackGameplayAbility::UpdateFacingToTarget() const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !Avatar->Implements<UCombatInterface>())
	{
		return;
	}

	// 对应蓝图里：Get Target Location → Get Avatar Actor from Actor Info → Update Facing Vector
	ICombatInterface::Execute_UpdateFacingVector(Avatar, GetTargetLocation());
}

UAbilityTask_PlayMontageAndWaitForEvent* UAuraAttackGameplayAbility::PlayAttackMontageAndWaitForEvent()
{
	// 缓存本次选中的蒙太奇数据，供蓝图读取（Montage / NotifyTag / AttackMethodTag）
	CachedTaggedMontage = GetRandomAttackMontage();
	if (!CachedTaggedMontage.Montage)
	{
		// 兜底：没有可用蒙太奇（未实现 GetAttackMontages / 蒙太奇数组为空）时不会生成任何 Task，
		// 也就不会有任何终止回调来结束能力，会永久卡在 Active（且冷却已提交，等于再也放不出攻击）。
		// 这里直接按取消语义结束本次激活，并返回 nullptr 交给蓝图做提示。
		ABILITY_LOG(Warning, TEXT("%s: 没有可用的攻击蒙太奇，已结束本次激活"), *GetName());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return nullptr;
	}

	UAbilityTask_PlayMontageAndWaitForEvent* Task = UAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(
		this,
		NAME_None,
		CachedTaggedMontage.Montage,
		CachedTaggedMontage.NotifyTag,
		1.f);

	if (!Task)
	{
		// 同理兜底：Task 创建失败（例如能力未实例化）时同样不会有终止回调
		ABILITY_LOG(Warning, TEXT("%s: 创建 PlayMontageAndWaitForEvent Task 失败，已结束本次激活"), *GetName());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return nullptr;
	}

	// 蒙太奇 完成/中断/取消 三个终止状态自动结束能力，无需在蓝图里再连 EndAbility
	Task->OnCompleted.AddDynamic(this, &UAuraAttackGameplayAbility::EndAbilityOnMontageTerminal);
	Task->OnInterrupted.AddDynamic(this, &UAuraAttackGameplayAbility::EndAbilityOnMontageTerminal);
	Task->OnCancelled.AddDynamic(this, &UAuraAttackGameplayAbility::EndAbilityOnMontageTerminal);

	return Task;
}

FVector UAuraAttackGameplayAbility::GetNotifyLocation() const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (Avatar->Implements<UCombatInterface>())
		{
			return ICombatInterface::Execute_GetCombatSocketLocation(Avatar, CachedTaggedMontage.AttackMethodTag);
		}
	}
	return FVector::ZeroVector;
}

USoundBase* UAuraAttackGameplayAbility::GetHitSound() const
{
	return CachedTaggedMontage.HitSound;
}

UAbilityTask_PlayMontageAndWaitForEvent* UAuraAttackGameplayAbility::PerformAttack()
{
	// 0. 先提交冷却：内部会 CheckCooldown（判断冷却 GE 授予的 Tag 是否在身上）
	//    仍在冷却中返回 false —— 此时直接结束本次激活，既不朝向目标也不播放蒙太奇
	//    ForceCooldown = false：走正常的冷却检查；冷却 GE 由 Class Defaults 的 Cooldown Gameplay Effect Class 指定
	if (!CommitAbilityCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return nullptr;
	}

	// 1. 先朝向目标
	UpdateFacingToTarget();
	// 2. 播放蒙太奇并等待事件；命中位置在 EventReceived 里通过 GetNotifyLocation() 实时获取
	return PlayAttackMontageAndWaitForEvent();
}

void UAuraAttackGameplayAbility::EndAbilityOnMontageTerminal(FGameplayEventData EventData)
{
	// 终止状态下结束能力（bReplicatedEndAbility = true, bWasCancelled = true 表示取消语义）
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
