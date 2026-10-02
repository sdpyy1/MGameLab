// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AuraAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AuraGameplayTags.h"
#include <Interaction/CombatInterface.h>
#include "kismet/GameplayStatics.h"
#include <Player/AuraPlayerController.h>
#include <GameFramework/Character.h>
UAuraAttributeSet::UAuraAttributeSet()
{
	/*
		还可以在GameState中的ASC中用TableData进行的初始化，
		或者直接创建一种用于初始化的GameplayEffect，	创建角色时，施加给角色即可
	*/
	//InitHealth(100.f);
	//InitMana(50.f);

	const FAuraGameplayTags& tags = FAuraGameplayTags::Get();

	TagsToAttributesMap.Add(tags.Attributes_Primary_Strength, GetStrengthAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Primary_Resilience, GetResilienceAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Primary_Vigor, GetVigorAttribute);

	TagsToAttributesMap.Add(tags.Attributes_Secondary_Armor, GetArmorAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Secondary_ArmorPenetration, GetArmorPenetrationAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Secondary_BlockChance, GetBlockChanceAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Secondary_CriticalHitChance, GetCriticalHitChanceAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Secondary_CriticalHitDamage, GetCriticalHitDamageAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Secondary_CriticalHitResistance, GetCriticalHitResistanceAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Secondary_HealthRegeneration, GetHealthRegenerationAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Secondary_ManaRegeneration, GetManaRegenerationAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributesMap.Add(tags.Attributes_Secondary_MaxMana, GetMaxManaAttribute);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	/*
		注意这里只是设置当前帧处理的数值，并不会写回AttributeSet中，需要在PostGameplayEffectExecute进行写回
	*/
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		if (GetHealth() <= 0.f) {
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Data.Target.GetAvatarActor())) {
				CombatInterface->Die();
			}
		}
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		float IncomingDamageCache = GetIncomingDamage();
		SetIncomingDamage(0.f);
		if (IncomingDamageCache > 0.f) {
			const float NewHealth = GetHealth() - IncomingDamageCache;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
			const bool bFatal = NewHealth <= 0.f;  // Death
			if (bFatal) {// 致死伤害
				if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Data.Target.GetAvatarActor())) {
					CombatInterface->Die();
				}
			}
			else {// 非致死伤害，激活HitReact能力（会播放受击动画）
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAuraGameplayTags::Get().Effects_HitReact);
				Data.Target.TryActivateAbilitiesByTag(TagContainer);
			}
		}

		if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(UGameplayStatics::GetPlayerController(Data.Target.GetAvatarActor(), 0))) {
			AuraPlayerController->ShowDamageNumber(IncomingDamageCache, CastChecked<ACharacter>(Data.Target.GetAvatarActor()));
		}
	}
}
