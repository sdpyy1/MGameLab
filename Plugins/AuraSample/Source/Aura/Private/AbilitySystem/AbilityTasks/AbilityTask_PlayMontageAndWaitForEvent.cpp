// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AbilityTasks/AbilityTask_PlayMontageAndWaitForEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/Ability/AuraAttackGameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"

UAbilityTask_PlayMontageAndWaitForEvent::UAbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Rate = 1.f;
	bStopWhenAbilityEnds = true;
}

UAbilityTask_PlayMontageAndWaitForEvent* UAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(
	UGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	UAnimMontage* MontageToPlay,
	FGameplayTag EventTag,
	float Rate,
	FName StartSection,
	bool bStopWhenAbilityEnds,
	float AnimRootMotionTranslationScale,
	float StartTimeSeconds)
{
	UAbilityTask_PlayMontageAndWaitForEvent* Task = NewAbilityTask<UAbilityTask_PlayMontageAndWaitForEvent>(OwningAbility, TaskInstanceName);
	if (Task)
	{
		Task->MontageToPlay = MontageToPlay;
		Task->EventTag = EventTag;
		Task->Rate = Rate;
		Task->StartSection = StartSection;
		Task->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
		Task->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
		Task->StartTimeSeconds = StartTimeSeconds;
	}
	return Task;
}

UAbilityTask_PlayMontageAndWaitForEvent* UAbilityTask_PlayMontageAndWaitForEvent::PerformAttack(UGameplayAbility* OwningAbility)
{
	UAuraAttackGameplayAbility* AttackAbility = Cast<UAuraAttackGameplayAbility>(OwningAbility);
	if (!AttackAbility)
	{
		return nullptr;
	}

	// 实际逻辑在 GA 侧：提交冷却 → 朝向目标 → 随机蒙太奇（缓存）→ 播放并等待 NotifyTag 事件
	// 命中位置不再通过本函数暴露，改由 GA::GetNotifyLocation() 在 EventReceived 帧实时获取
	return AttackAbility->PerformAttack();
}

UAnimInstance* UAbilityTask_PlayMontageAndWaitForEvent::GetAnimInstance() const
{
	if (AbilitySystemComponent.IsValid())
	{
		const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();
		if (ActorInfo)
		{
			return ActorInfo->GetAnimInstance();
		}
	}
	return nullptr;
}

void UAbilityTask_PlayMontageAndWaitForEvent::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	bool bPlayedMontage = false;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
		if (AnimInstance != nullptr)
		{
			if (ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection, StartTimeSeconds) > 0.f)
			{
				// 播放蒙太奇可能回调进游戏代码并销毁本能力，挂起时提前返回
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				// 能力被打断时收到通知
				InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UAbilityTask_PlayMontageAndWaitForEvent::OnGameplayAbilityCancelled);

				// 绑定蒙太奇回调
				BlendingOutDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				// 注册要等待的 GameplayEvent（单 Tag，与 OnDestroy 中的注销严格对称）
				GameplayEventDelegateHandle = ASC->AddGameplayEventTagContainerDelegate(
					FGameplayTagContainer(EventTag),
					FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent));

				// 根位移缩放（对齐引擎 PlayMontageAndWait）
				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
		else
		{
			ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWaitForEvent: no valid AnimInstance, PlayMontage failed!"));
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWaitForEvent called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWaitForEvent failed to play montage %s in Ability %s."), *GetNameSafe(MontageToPlay), *GetNameSafe(Ability));
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayEventData());
		}
	}

	SetWaitingOnAvatar();
}

void UAbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	const bool bPlayingThisMontage = (Montage == MontageToPlay) && Ability && Ability->GetCurrentMontage() == MontageToPlay;
	if (bPlayingThisMontage)
	{
		// 还原根位移缩放
		ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
		if (Character && (Character->GetLocalRole() == ROLE_Authority ||
			(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
		{
			Character->SetAnimRootMotionTranslationScale(1.f);
		}

		if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
		{
			ASC->ClearAnimatingAbility(Ability);
		}
	}

	if (bInterrupted)
	{
		BroadcastInterrupted();
	}
	else if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnBlendOut.Broadcast(FGameplayEventData());
	}
}

void UAbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast(FGameplayEventData());
		}
		EndTask();
	}
	else
	{
		BroadcastInterrupted();
	}
}

void UAbilityTask_PlayMontageAndWaitForEvent::BroadcastInterrupted()
{
	// BlendingOut 与 Ended 都可能报告打断，这里保证只广播一次
	if (!bInterruptBroadcasted && ShouldBroadcastAbilityTaskDelegates())
	{
		bInterruptBroadcasted = true;
		OnInterrupted.Broadcast(FGameplayEventData());
	}
	EndTask();
}

void UAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent(FGameplayTag MatchingTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempData = Payload ? *Payload : FGameplayEventData();
		TempData.EventTag = MatchingTag;
		EventReceived.Broadcast(TempData);
	}
}

void UAbilityTask_PlayMontageAndWaitForEvent::OnGameplayAbilityCancelled()
{
	// 能力被打断：停掉蒙太奇并按“中断”语义通知（而非 OnCancelled）
	StopPlayingMontage();
	BroadcastInterrupted();
}

void UAbilityTask_PlayMontageAndWaitForEvent::ExternalCancel()
{
	// 外部显式取消才走 OnCancelled
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCancelled.Broadcast(FGameplayEventData());
	}
	Super::ExternalCancel();
}

bool UAbilityTask_PlayMontageAndWaitForEvent::StopPlayingMontage()
{
	if (Ability == nullptr)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (ActorInfo == nullptr)
	{
		return false;
	}

	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// 仅当本能力仍在播放该蒙太奇时才停止（能力被打断时应自动停止蒙太奇）
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && Ability)
	{
		if (ASC->GetAnimatingAbility() == Ability
			&& ASC->GetCurrentMontage() == MontageToPlay)
		{
			// 解绑蒙太奇委托，避免停止时再触发 OnMontageEnded 造成重复广播
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			ASC->CurrentMontageStop();
			return true;
		}
	}

	return false;
}

FString UAbilityTask_PlayMontageAndWaitForEvent::GetDebugString() const
{
	UAnimInstance* AnimInstance = GetAnimInstance();
	if (AnimInstance && MontageToPlay && AnimInstance->Montage_IsPlaying(MontageToPlay))
	{
		return FString::Printf(
			TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  Section: %s  EventTag: %s"),
			*GetNameSafe(MontageToPlay),
			*StartSection.ToString(),
			*EventTag.ToString());
	}
	return FString::Printf(TEXT("PlayMontageAndWaitForEvent. %s"), *InstanceName.ToString());
}

void UAbilityTask_PlayMontageAndWaitForEvent::OnDestroy(bool AbilityEnded)
{
	// OnGameplayAbilityCancelled 是多播委托，需要移除；蒙太奇 End 委托非多播，随下个蒙太奇自动清理
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(InterruptedHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}

	// 注销 GameplayEvent 监听（与 Activate 中的注册使用同一 Tag，严格对称）
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		if (GameplayEventDelegateHandle.IsValid())
		{
			ASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(EventTag), GameplayEventDelegateHandle);
		}
	}

	Super::OnDestroy(AbilityEnded);
}
