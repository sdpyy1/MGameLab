// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include <AbilitySystem/AuraAttributeSet.h>
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include "Aura.h"
#include "AuraGameplayTags.h"
#include "Engine/DataTable.h"

void UAuraOverlayWidgetController::BroadcastInitialValues()
{
	const UAuraAttributeSet* attributeSet = AttributeSet;

	// Broadcast Delegate
	OnHealthChanged.Broadcast(attributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(attributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(attributeSet->GetMana());
	OnMaxManaChanged.Broadcast(attributeSet->GetMaxMana());
}

void UAuraOverlayWidgetController::BindCallbacksToDependencies()
{
	/* 纯 C++ 运行时没有 BP 帮忙配 DataTable，这里兜底加载一次 */
	if (!MessageWidgetDataTable)
	{
	MessageWidgetDataTable = LoadObject<UDataTable>(nullptr, TEXT("/AuraSample/Aura/UI/Config/DT_MessageWidgetData"));
	}

	const UAuraAttributeSet* attributeSet = AttributeSet;
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(attributeSet->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {	OnHealthChanged.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(attributeSet->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {	OnMaxHealthChanged.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(attributeSet->GetManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {	OnManaChanged.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(attributeSet->GetMaxManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {	OnMaxManaChanged.Broadcast(Data.NewValue); });
	
	// 需要屏幕提示的Effect，可以在AssetTag中加一条Message.xxx
	AbilitySystemComponent->EffectAssetTagsDelegate.AddLambda([this](const FGameplayTagContainer& TagContainer) {
		for (const FGameplayTag& Tag : TagContainer) {
			/* Message 是父 Tag，下面挂着 Message.FireAttack / HealthPotion / ManaPotion */
			const FGameplayTag MessageTag = FAuraGameplayTags::Get().Message;

			if (Tag.MatchesTag(MessageTag)) {
				/* 表里找不到对应行就跳过，不要因为少配一行提示就崩 */
				if (const FUIWidgetRow* Row = GetDataRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag)) {
					MessageWidgetRowDelegate.Broadcast(*Row);
				}
			}
		}
		});
}
