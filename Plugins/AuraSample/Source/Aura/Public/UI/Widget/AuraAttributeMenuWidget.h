// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraAttributeMenuWidget.generated.h"

class UVerticalBox;
class UScrollBox;
class UAuraAttributeRowWidget;
struct FAuraAttributeInfo;

/**
 *	属性菜单（纯 C++ 搭建，只读）。
 *
 *	打开方式：屏幕右下角的"属性菜单"按钮。
 *	关闭方式：右上角按钮 / 点击面板外的遮罩。
 *
 *	行内容来自 UAuraAttributeMenuController 广播的 FAuraAttributeInfo，
 *	按 Attributes.Primary / Secondary / Vital 自动分组，组内按 Tag 名排序。
 */
UCLASS()
class AURA_API UAuraAttributeMenuWidget : public UAuraUserWidget
{
	GENERATED_BODY()

protected:
	virtual void BuildWidgetTree() override;
	virtual bool BindWidgetDelegates() override;

	UFUNCTION() void HandleAttributeInfo(const FAuraAttributeInfo& Info);
	UFUNCTION() void HandleCloseClicked();

	/** 按分类把收集到的属性排好序，重建整个列表（只在出现新属性时调用） */
	void RebuildAttributeList();

	/** 插入一个分组标题 + 分隔线 */
	void AddSectionHeader(const FText& Title);

	UPROPERTY() TObjectPtr<UVerticalBox> AttributeList;
	UPROPERTY() TObjectPtr<UScrollBox> AttributeScroll;

	/* Tag → 行控件：属性数值变化时只刷新对应那一行 */
	UPROPERTY() TMap<FGameplayTag, TObjectPtr<UAuraAttributeRowWidget>> RowWidgets;

	/* 已经收到的属性（新属性到来时才重建列表，避免频繁重建） */
	TArray<FAuraAttributeInfo> CachedAttributes;
};
