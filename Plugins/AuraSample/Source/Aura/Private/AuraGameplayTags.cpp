// Fill out your copyright notice in the Description page of Project Settings.

#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"
FAuraGameplayTags FAuraGameplayTags::GameplayTags;

void FAuraGameplayTags::InitNativeGameplayTags()
{
	/*
	* Primary Attributes
	*/
	GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Strength"),
		FString("Increases physical damage")
	);

	GameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Intelligence"),
		FString("Increases magical damage")
	);

	GameplayTags.Attributes_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Resilience"),
		FString("Increases Armor and Armor Penetration")
	);

	GameplayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Vigor"),
		FString("Increases Health")
	);

	/*
	 * Secondary Attributes
	 */

	GameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Armor"),
		FString("Reduces damage taken, improves Block Chance")
	);

	GameplayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ArmorPenetration"),
		FString("Ignores Percentage of enemy Armor, increases Critical Hit Chance")
	);

	GameplayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.BlockChance"),
		FString("Chance to cut incoming damage in half")
	);

	GameplayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitChance"),
		FString("Chance to double damage plus critical hit bonus")
	);

	GameplayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitDamage"),
		FString("Bonus damage added when a critical hit is scored")
	);

	GameplayTags.Attributes_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitResistance"),
		FString("Reduces Critical Hit Chance of attacking enemies")
	);

	GameplayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.HealthRegeneration"),
		FString("Amount of Health regenerated every 1 second")
	);

	GameplayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ManaRegeneration"),
		FString("Amount of Mana regenerated every 1 second")
	);

	GameplayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxHealth"),
		FString("Maximum amount of Health obtainable")
	);

	GameplayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxMana"),
		FString("Maximum amount of Mana obtainable")
	);

	/*
	 * Vital Attributes
	 */

	GameplayTags.Attributes_Vital_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.Health"),
		FString("当前生命值")
	);

	GameplayTags.Attributes_Vital_Mana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.Mana"),
		FString("当前魔力值")
	);

	/*
	 * Input Tags
	 */

	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"),
		FString("Input Tag for Left Mouse Button")
	);

	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"),
		FString("Input Tag for Right Mouse Button")
	);

	GameplayTags.InputTag_Q = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Q"),
		FString("Input Tag for Q key")
	);

	GameplayTags.InputTag_W = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.W"),
		FString("Input Tag for W key")
	);

	GameplayTags.InputTag_E = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.E"),
		FString("Input Tag for E key")
	);

	GameplayTags.InputTag_R = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.R"),
		FString("Input Tag for R key")
	);

	GameplayTags.InputTag_A = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.A"),
		FString("Input Tag for A key")
	);

	GameplayTags.InputTag_S = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.S"),
		FString("Input Tag for S key")
	);

	GameplayTags.InputTag_D = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.D"),
		FString("Input Tag for D key")
	);

	GameplayTags.InputTag_F = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.F"),
		FString("Input Tag for F key")
	);
	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage"),
		FString("Damage")
	);
	GameplayTags.Effects_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effects.HitReact"),
		FString("Enemy Use Hit React")
	);
	GameplayTags.Ability_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Attack"),
		FString("Ability Attack")
	);
	GameplayTags.Ability_CoolDown = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.CoolDown"),
		FString("Ability CoolDown")
	);

	GameplayTags.AttackMethod_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("AttackMethod.Weapon"),
		FString("Weapon Attack")
	);

	GameplayTags.AttackMethod_LeftHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("AttackMethod.LeftHand"),
		FString("Left Hand Attack")
	);

	GameplayTags.AttackMethod_RightHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("AttackMethod.RightHand"),
		FString("Right Hand Attack")
	);

	GameplayTags.AttackMethod_Tail = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("AttackMethod.Tail"),
		FString("Tail Attack")
	);

	/*
	 * Montage Notify Tag（蒙太奇 Notify 通知能力用；能力侧用 WaitGameplayEvent 接收）
	 * 统一使用一个 Tag，由具体攻击能力在 GA 中配置/继承默认。
	 */
	GameplayTags.Montage_Notify_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Notify.Attack"),
		FString("攻击蒙太奇事件（统一使用，能力侧用 WaitGameplayEvent 接收）")
	);
	
	GameplayTags.Montage_Notify_Summon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Notify.Summon"),
		FString("Summon召唤Tag，用于蒙太奇渲染Niagara召唤法阵")
	);
	

	/*
	 * UI Message Tags（Overlay 显示提示用）
	 */
	GameplayTags.Message = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Message"),
		FString("消息类 Tag 的父标签")
	);

	GameplayTags.Message_FireAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Message.FireAttack"),
		FString("火焰区域造成伤害的提示")
	);

	GameplayTags.Message_HealthPotion = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Message.HealthPotion"),
		FString("恢复生命药水的提示")
	);

	GameplayTags.Message_ManaPotion = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Message.ManaPotion"),
		FString("恢复魔力药水的提示")
	);
}