// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AbilityTask_PlayMontageAndWaitForEvent.generated.h"

class UAnimInstance;
class UGameplayAbility;

// 广播给蓝图的回调：携带触发时的 GameplayEventData（按值传递，反射不允许暴露 FGameplayEventData*）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayMontageAndWaitForEventDelegate, FGameplayEventData, EventData);

/**
 * 同时“播放蒙太奇”并在播放期间“等待指定 GameplayEvent”的能力任务：
 * - 蒙太奇播放结束 / 被打断 / 被外部取消，分别广播 OnCompleted / OnInterrupted / OnCancelled
 * - 期间若收到 EventTag 的 GameplayEvent，则广播 EventReceived（通常在这里做伤害判定等）
 * 对应蓝图里把 PlayMontageAndWait 与 WaitGameplayEvent 串起来的常见写法。
 * 语义对齐引擎 UAbilityTask_PlayMontageAndWait + UAbilityTask_WaitGameplayEvent。
 */
UCLASS()
class AURA_API UAbilityTask_PlayMontageAndWaitForEvent : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer);

	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual FString GetDebugString() const override;
	virtual void OnDestroy(bool AbilityEnded) override;

	// 蒙太奇 完成/中断/取消 等终止态委托：C++ 内部已绑定自动 EndAbility（PlayAttackMontageAndWaitForEvent
	// 里 AddDynamic），蓝图可按需再接自己的逻辑（多播，两者都会触发）。
	/** 蒙太奇正常播放结束 */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnCompleted;

	/** 蒙太奇开始混合退出（未被打断） */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnBlendOut;

	/** 蒙太奇 / 能力被打断 */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnInterrupted;

	/** 能力被外部显式取消 */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnCancelled;

	/** 收到了目标 GameplayEvent，交给你后续处理（蓝图唯一需要连接的输出） */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate EventReceived;

	/**
	 * 播放蒙太奇并等待事件。
	 * @param EventTag 需要监听的 GameplayEvent Tag（单个，与注销严格对称）。
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayMontageAndWaitForEvent* PlayMontageAndWaitForEvent(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		UAnimMontage* MontageToPlay,
		FGameplayTag EventTag,
		float Rate = 1.f,
		FName StartSection = NAME_None,
		bool bStopWhenAbilityEnds = true,
		float AnimRootMotionTranslationScale = 1.f,
		float StartTimeSeconds = 0.f);

	/**
	 * 攻击一步到位（蓝图异步节点入口）：朝向目标 → 随机播放攻击蒙太奇 → 等待其 NotifyTag 事件。
	 * 蓝图节点会自动展开 EventReceived / OnCompleted / OnInterrupted / OnCancelled 等输出引脚。
	 * 注意：异步工厂必须定义在 UBlueprintAsyncActionBase 子类内部，蓝图才会把它注册成异步节点，
	 * 所以这里只是转发入口，实际逻辑在 UAuraAttackGameplayAbility::PerformAttack 中。
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayMontageAndWaitForEvent* PerformAttack(UGameplayAbility* OwningAbility);

private:
	UAnimMontage* MontageToPlay = nullptr;
	FGameplayTag EventTag;
	float Rate = 1.f;
	FName StartSection = NAME_None;
	float AnimRootMotionTranslationScale = 1.f;
	float StartTimeSeconds = 0.f;
	bool bStopWhenAbilityEnds = true;
	/** 防止打断事件经 BlendingOut 与 Ended 两条路径重复广播 */
	bool bInterruptBroadcasted = false;

	UAnimInstance* GetAnimInstance() const;
	bool StopPlayingMontage();
	void BroadcastInterrupted();

	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnGameplayEvent(FGameplayTag MatchingTag, const FGameplayEventData* Payload);
	void OnGameplayAbilityCancelled();

	FOnMontageEnded MontageEndedDelegate;
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FDelegateHandle InterruptedHandle;
	FDelegateHandle GameplayEventDelegateHandle;
};
