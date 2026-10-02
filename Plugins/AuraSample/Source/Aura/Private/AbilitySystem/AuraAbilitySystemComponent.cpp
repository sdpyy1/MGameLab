// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura.h"
#include <AbilitySystem/Ability/AuraGameplayAbility.h>
#include "AbilitySystem/Ability/AuraAttackGameplayAbility.h"
#include "AuraGameplayTags.h"
#include <GameplayEffect.h>
void UAuraAbilitySystemComponent::AbilityCharacterInfoSet()
{
	// 绑定AbilitySystemComponent自带的委托
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		UAuraGameplayAbility* AuraGameplayAbility = AbilityClass.Get()->GetDefaultObject<UAuraGameplayAbility>();
		if (AuraGameplayAbility)
		{
			if (AuraGameplayAbility->InputMappingTag.IsValid())
			{
				AbilitySpec.DynamicAbilityTags.AddTag(AuraGameplayAbility->InputMappingTag);
			}

			// 2) 攻击类能力：直接写入 Ability.Attack 到 AssetTags，供 TryActivateAbilitiesByTag 按此 Tag 激活
			if (Cast<UAuraAttackGameplayAbility>(AuraGameplayAbility))
			{
				const FGameplayTag AttackTag = FAuraGameplayTags::Get().Ability_Attack;
				// AbilityTags 为 5.5 起 deprecated 成员，此处用 UE 宏屏蔽告警；仅在尚未包含时添加，避免重复写入
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				if (!AuraGameplayAbility->AbilityTags.HasTag(AttackTag))
				{
					AuraGameplayAbility->AbilityTags.AddTag(AttackTag);
				}
				PRAGMA_ENABLE_DEPRECATION_WARNINGS
			}

			GiveAbility(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float InLevel)
{
	check(IsValid(GameplayEffectClass));

	FGameplayEffectContextHandle ContextHandle = MakeEffectContext();
	ContextHandle.AddSourceObject(GetAvatarActor());
	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(GameplayEffectClass, InLevel, ContextHandle);
	ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

// Input会触发一个Tag，Tag会在这里激活对应的能力，只是给player用的
void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& Tag)
{
	if (!Tag.IsValid()) return;
	for (auto& AbilitySpec : GetActivatableAbilities()) {
			if (AbilitySpec.DynamicAbilityTags.HasTagExact(Tag)) {
			AbilitySpecInputPressed(AbilitySpec); // 只是激活一个AbilitySpec中的Bool
			if (!AbilitySpec.IsActive()) {
				TryActivateAbility(AbilitySpec.Handle); // 如果按下的Ability没有激活就激活他
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagRelease(const FGameplayTag& Tag)
{
	if (!Tag.IsValid()) return;
	for (auto& AbilitySpec : GetActivatableAbilities()) {
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(Tag)) {
			AbilitySpecInputReleased(AbilitySpec); // 只告知ReleaseBool，因为有些Ability并不是松开就失效
		}

	}
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer tagContainer;
	EffectSpec.GetAllAssetTags(tagContainer);
	EffectAssetTagsDelegate.Broadcast(tagContainer);
}
