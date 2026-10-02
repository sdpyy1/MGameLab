// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraAttributeRowWidget.generated.h"

class UTextBlock;
class UBorder;
struct FAuraAttributeInfo;

/**
 *	属性菜单里的一行：左边"名称 + 说明"，右边数值。
 */
UCLASS()
class AURA_API UAuraAttributeRowWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	/** 更新这一行的内容（控件树还没搭好时会先缓存，等构造完再写进去） */
	void SetAttributeInfo(const FAuraAttributeInfo& InInfo);

protected:
	virtual void BuildWidgetTree() override;
	virtual void NativeConstruct() override;

	/** 把缓存的数据写进控件 */
	void ApplyAttributeInfo();

	/** 鼠标移入高亮，让列表更好读 */
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UPROPERTY() TObjectPtr<UBorder> RowBackground;
	UPROPERTY() TObjectPtr<UTextBlock> NameText;
	UPROPERTY() TObjectPtr<UTextBlock> DescriptionText;
	UPROPERTY() TObjectPtr<UTextBlock> ValueText;

	/** 缓存的数据（FAuraAttributeInfo 是 USTRUCT，不能直接做 UPROPERTY 存引用，这里存副本） */
	FText CachedName;
	FText CachedDescription;
	float CachedValue = 0.f;
	bool bHasInfo = false;
};
