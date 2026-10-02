// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/AuraGameplayAbility.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/EnemyInterface.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AbilityTasks/AbilityTask_PlayMontageAndWaitForEvent.h"
#include "Sound/SoundBase.h"
#include "AuraAttackGameplayAbility.generated.h"

class UAbilityTask_PlayMontageAndWaitForEvent;

UCLASS()
class AURA_API UAuraAttackGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
public:
	// 获取目标位置：玩家用鼠标落点，敌人通过 EnemyInterface 取战斗目标位置
	UFUNCTION(BlueprintPure, Category = "Montage")
	FVector GetTargetLocation() const;

	// 获取当前战斗目标：通过 EnemyInterface::GetCombatTarget 取得（未实现接口或没有目标时返回 nullptr）
	UFUNCTION(BlueprintPure, Category = "Montage")
	AActor* GetCombatTarget() const;

	// 调整角色朝向：取 GetTargetLocation() 作为朝向目标，
	// 通过 CombatInterface::UpdateFacingVector（蓝图实现）让 Avatar 面向它
	UFUNCTION(BlueprintCallable, Category = "Montage")
	void UpdateFacingToTarget() const;

	// 从角色的攻击蒙太奇列表中随机选取一个返回（含 Montage / NotifyTag / AttackMethodTag）
	UFUNCTION(BlueprintPure, Category = "Montage")
	FTaggedMontage GetRandomAttackMontage() const;

	// 缓存：本次攻击实际选中的蒙太奇数据（Montage / NotifyTag / AttackMethodTag），蓝图可直接读取其字段
	UPROPERTY(BlueprintReadOnly, Category = "Montage")
	FTaggedMontage CachedTaggedMontage;

	/**
	 * 实时获取命中位置：以 CachedTaggedMontage.AttackMethodTag 经 CombatInterface::GetCombatSocketLocation
	 * 取到的武器/手部 Socket 世界坐标。请在 EventReceived（Notify 命中帧）里调用——此时动画已播放到该帧，
	 * 取到的是手部“当前”位置，比能力激活时算一次更准确。
	 */
	UFUNCTION(BlueprintPure, Category = "Montage")
	FVector GetNotifyLocation() const;

	// 获取当前缓存蒙太奇（CachedTaggedMontage）的命中音效
	UFUNCTION(BlueprintPure, Category = "Montage")
	USoundBase* GetHitSound() const;

	// 封装：随机取一个攻击蒙太奇并播放，同时等待该蒙太奇 NotifyTag 的 GameplayEvent。
	// 完成/中断/取消 三个终止态自动 EndAbility；收到 NotifyTag 事件时通过 Task->EventReceived 交回蓝图。
	// 失败兜底：没有可用蒙太奇或 Task 创建失败时，直接 EndAbility 并返回 nullptr，不会把能力卡在 Active。
	UFUNCTION(BlueprintCallable, Category = "Montage")
	UAbilityTask_PlayMontageAndWaitForEvent* PlayAttackMontageAndWaitForEvent();

	// 最终攻击封装（C++ 内部入口）：先提交冷却（CommitAbilityCooldown），冷却中直接结束激活并返回 nullptr；
	// 通过后再朝向目标，最后播放蒙太奇（内部已含等待 NotifyTag 事件与自动 EndAbility）。
	// 注意：异步工厂必须定义在 Task 类（UBlueprintAsyncActionBase 子类）内部才会被蓝图识别为异步节点，
	// 因此蓝图入口是 UAbilityTask_PlayMontageAndWaitForEvent::PerformAttack（静态工厂），由它转发到这里。
	UAbilityTask_PlayMontageAndWaitForEvent* PerformAttack();

private:
	UFUNCTION()
	void EndAbilityOnMontageTerminal(FGameplayEventData EventData);
};
