// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AuraAttributeRowWidget.h"

#include "UI/AuraUIStyle.h"
#include "UI/AuraAttributeText.h"
#include "AbilitySystem/Data/AttributeInfo.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UAuraAttributeRowWidget::BuildWidgetTree()
{
	RowBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RowBackground"));
	RowBackground->SetBrush(FAuraUIStyle::Rounded(FAuraUIStyle::RowFill(), 7.f));
	RowBackground->SetPadding(FMargin(14.f, 9.f));
	RowBackground->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = RowBackground;

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	RowBackground->SetContent(Row);

	/* 左边：名称 + 说明 */
	{
		UVerticalBox* LeftColumn = WidgetTree->ConstructWidget<UVerticalBox>();

		NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NameText"));
		FAuraUIStyle::StyleText(NameText, 16, FAuraUIStyle::TextPrimary(), true);
		LeftColumn->AddChildToVerticalBox(NameText)->SetPadding(FMargin(0.f, 0.f, 0.f, 2.f));

		DescriptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DescriptionText"));
		FAuraUIStyle::StyleText(DescriptionText, 12, FAuraUIStyle::TextMuted(), false);
		LeftColumn->AddChildToVerticalBox(DescriptionText);

		/* 让左列吃掉所有剩余宽度，把数值挤到最右边 */
		UHorizontalBoxSlot* LeftSlot = Row->AddChildToHorizontalBox(LeftColumn);
		LeftSlot->SetVerticalAlignment(VAlign_Center);
		LeftSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	/* 右边：数值 */
	{
		ValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ValueText"));
		FAuraUIStyle::StyleText(ValueText, 19, FAuraUIStyle::Accent(), true);
		ValueText->SetJustification(ETextJustify::Right);

		UHorizontalBoxSlot* ValueSlot = Row->AddChildToHorizontalBox(ValueText);
		ValueSlot->SetVerticalAlignment(VAlign_Center);
		ValueSlot->SetPadding(FMargin(16.f, 0.f, 2.f, 0.f));
	}

	/*
		UUserWidget 默认是 SelfHitTestInvisible，收不到鼠标事件，
		这里显式设成 Visible，NativeOnMouseEnter / Leave 才会触发（列表悬停高亮）。
	*/
	SetVisibility(ESlateVisibility::Visible);
}

void UAuraAttributeRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyAttributeInfo();
}

void UAuraAttributeRowWidget::SetAttributeInfo(const FAuraAttributeInfo& InInfo)
{
	/*
		名称 / 说明优先用 AuraAttributeText 里的中文文案；
		表里没覆盖到的 Tag 退回 DA_AttributeInfo 里配的文本；
		再没有就用 Tag 最后一段兜底，避免出现空白行。
	*/
	CachedName = AuraAttributeText::GetDisplayName(InInfo.AttributeTag);
	if (CachedName.IsEmpty())
	{
		CachedName = InInfo.AttributeName;
	}
	if (CachedName.IsEmpty())
	{
		CachedName = AuraAttributeText::MakeFallbackName(InInfo.AttributeTag);
	}

	CachedDescription = AuraAttributeText::GetDescription(InInfo.AttributeTag);
	if (CachedDescription.IsEmpty())
	{
		CachedDescription = InInfo.AttributeDescription;
	}

	CachedValue = InInfo.AttributeValue;
	bHasInfo = true;

	ApplyAttributeInfo();
}

void UAuraAttributeRowWidget::ApplyAttributeInfo()
{
	if (!bHasInfo) return;

	if (NameText) NameText->SetText(CachedName);
	if (DescriptionText) DescriptionText->SetText(CachedDescription);
	if (ValueText)
	{
		ValueText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), CachedValue)));
	}
}

void UAuraAttributeRowWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (RowBackground)
	{
		RowBackground->SetBrush(FAuraUIStyle::Rounded(FLinearColor(1.f, 1.f, 1.f, 0.09f), 7.f));
	}
}

void UAuraAttributeRowWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (RowBackground)
	{
		RowBackground->SetBrush(FAuraUIStyle::Rounded(FAuraUIStyle::RowFill(), 7.f));
	}
}
