// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 *	属性界面的中文文案表。
 *
 *	DA_AttributeInfo 里配的是英文（Intelligence / "Increases magical damage"…），
 *	这里统一覆盖一层中文；表里没覆盖到的 Tag 会自动退回 DataAsset 里配的文本。
 *	以后要改属性名 / 说明，只动这一张表就行。
 */
namespace AuraAttributeText
{
	struct FAttributeText
	{
		const TCHAR* Name;
		const TCHAR* Description;
	};

	/** Tag 名 → 中文名 + 中文说明 */
	inline const TMap<FName, FAttributeText>& GetTable()
	{
		static const TMap<FName, FAttributeText> Table =
		{
			/* ---- 主属性 ---- */
			{ TEXT("Attributes.Primary.Strength"),               { TEXT("力量"),         TEXT("提升物理伤害") } },
			{ TEXT("Attributes.Primary.Intelligence"),           { TEXT("智力"),         TEXT("提升魔法伤害") } },
			{ TEXT("Attributes.Primary.Resilience"),             { TEXT("韧性"),         TEXT("提升护甲与护甲穿透") } },
			{ TEXT("Attributes.Primary.Vigor"),                  { TEXT("体魄"),         TEXT("提升生命值") } },

			/* ---- 次要属性 ---- */
			{ TEXT("Attributes.Secondary.Armor"),                { TEXT("护甲"),         TEXT("减少受到的伤害，并提升格挡几率") } },
			{ TEXT("Attributes.Secondary.ArmorPenetration"),     { TEXT("护甲穿透"),     TEXT("按百分比无视敌人护甲，并提升暴击几率") } },
			{ TEXT("Attributes.Secondary.BlockChance"),          { TEXT("格挡几率"),     TEXT("有几率将受到的伤害减半") } },
			{ TEXT("Attributes.Secondary.CriticalHitChance"),    { TEXT("暴击几率"),     TEXT("有几率造成双倍伤害并附加暴击加成") } },
			{ TEXT("Attributes.Secondary.CriticalHitDamage"),    { TEXT("暴击伤害"),     TEXT("暴击时额外附加的伤害") } },
			{ TEXT("Attributes.Secondary.CriticalHitResistance"),{ TEXT("暴击抗性"),     TEXT("降低攻击者的暴击几率") } },
			{ TEXT("Attributes.Secondary.HealthRegeneration"),   { TEXT("生命回复"),     TEXT("每秒回复的生命值") } },
			{ TEXT("Attributes.Secondary.ManaRegeneration"),     { TEXT("法力回复"),     TEXT("每秒回复的法力值") } },
			{ TEXT("Attributes.Secondary.MaxHealth"),            { TEXT("生命上限"),     TEXT("可获得的生命值上限") } },
			{ TEXT("Attributes.Secondary.MaxMana"),              { TEXT("法力上限"),     TEXT("可获得的法力值上限") } },

			/* ---- 生命 / 法力（当前值，暂时没进属性菜单，先备着） ---- */
			{ TEXT("Attributes.Vital.Health"),                   { TEXT("生命值"),       TEXT("当前生命值") } },
			{ TEXT("Attributes.Vital.Mana"),                     { TEXT("法力值"),       TEXT("当前法力值") } },
		};

		return Table;
	}

	/** 找对应的中文文案；没有就返回 nullptr */
	inline const FAttributeText* Find(const FGameplayTag& AttributeTag)
	{
		if (!AttributeTag.IsValid()) return nullptr;
		return GetTable().Find(AttributeTag.GetTagName());
	}

	/** 属性名（中文）；没有对应项返回空 FText */
	inline FText GetDisplayName(const FGameplayTag& AttributeTag)
	{
		const FAttributeText* Entry = Find(AttributeTag);
		return Entry ? FText::FromString(Entry->Name) : FText();
	}

	/** 属性说明（中文）；没有对应项返回空 FText */
	inline FText GetDescription(const FGameplayTag& AttributeTag)
	{
		const FAttributeText* Entry = Find(AttributeTag);
		return Entry ? FText::FromString(Entry->Description) : FText();
	}

	/** 最后的兜底文案：取 Tag 的最后一段（例如 Attributes.Secondary.MaxHealth → MaxHealth） */
	inline FText MakeFallbackName(const FGameplayTag& AttributeTag)
	{
		const FString TagString = AttributeTag.ToString();

		int32 LastDotIndex = INDEX_NONE;
		if (TagString.FindLastChar(TEXT('.'), LastDotIndex))
		{
			return FText::FromString(TagString.RightChop(LastDotIndex + 1));
		}

		return FText::FromString(TagString);
	}
}
