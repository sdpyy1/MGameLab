// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/WidgetController/AuraAttributeMenuController.h"
#include <AbilitySystem/AuraAttributeSet.h>
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include "AbilitySystem/Data/AttributeInfo.h"
#include "AuraGameplayTags.h"
void UAuraAttributeMenuController::BroadcastInitialValues()
{
	/* 纯 C++ 运行时没有 BP 帮忙配 DataAsset，这里兜底加载一次 */
	if (!AttributeInfo)
	{
		AttributeInfo = LoadObject<UAttributeInfo>(nullptr, TEXT("/AuraSample/Aura/GamePlay/AbilitySystem/Config/DA_AttributeInfo"));
	}
	if (!AttributeInfo || !AttributeSet) return;

	UAuraAttributeSet* AS = AttributeSet;

	for (auto& Pair : AS->TagsToAttributesMap) {
		FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key);
		Info.AttributeValue = Pair.Value().GetNumericValue(AS);
		AttributeInfoDelegate.Broadcast(Info);
	}
}

void UAuraAttributeMenuController::BindCallbacksToDependencies()
{
	if (!AttributeInfo || !AttributeSet) return;

	UAuraAttributeSet* AS = AttributeSet;
	for (auto& Pair : AS->TagsToAttributesMap) {
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda([this, Pair, AS](const FOnAttributeChangeData& Data) {
			FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key);
			Info.AttributeValue = Pair.Value().GetNumericValue(AS);
			AttributeInfoDelegate.Broadcast(Info);
		});
	}
}
