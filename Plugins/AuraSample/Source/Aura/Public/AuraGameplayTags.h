// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
/*
	Global GameplayTags Manager
*/
	struct FAuraGameplayTags
{
public:
	static const FAuraGameplayTags& Get() { return GameplayTags; };
	static void InitNativeGameplayTags();

	/*
		Native Tags
	*/
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;

	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;

	/* Vital 属性（属性菜单 / DataTable 使用的 Tag） */
	FGameplayTag Attributes_Vital_Health;
	FGameplayTag Attributes_Vital_Mana;

	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_Q;
	FGameplayTag InputTag_W;
	FGameplayTag InputTag_E;
	FGameplayTag InputTag_R;
	FGameplayTag InputTag_A;
	FGameplayTag InputTag_S;
	FGameplayTag InputTag_D;
	FGameplayTag InputTag_F;

	FGameplayTag Damage;
	FGameplayTag Effects_HitReact;

	/* GameplayAblity会携带的Tag */
	FGameplayTag Ability_Attack;
	FGameplayTag Ability_CoolDown;

	/* 攻击方式：决定这次攻击从哪个部位/武器的 Socket 发出（武器 / 左手 / 右手 / 尾巴） */
	FGameplayTag AttackMethod_Weapon;
	FGameplayTag AttackMethod_LeftHand;
	FGameplayTag AttackMethod_RightHand;
	FGameplayTag AttackMethod_Tail;
	

	/* 蒙太奇 Notify 事件：蒙太奇里的 Notify 统一用这个 Tag 通知能力（能力侧用 WaitGameplayEvent 接收） */
	FGameplayTag Montage_Notify_Attack;
	FGameplayTag Montage_Notify_Summon;

	/*
		UI 消息：GameplayEffect 携带这些 AssetTag，Overlay 用它们去 DataTable 查提示并显示。
		Message 是父 Tag，用 Tag.MatchesTag(Message) 判断是否属于"消息"这一类。
	*/
	FGameplayTag Message;
	FGameplayTag Message_FireAttack;
	FGameplayTag Message_HealthPotion;
	FGameplayTag Message_ManaPotion;

protected:

private:
	static FAuraGameplayTags GameplayTags;
};