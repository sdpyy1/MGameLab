// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AuraAttributeMenuWidget.h"

#include "UI/AuraUIStyle.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraAttributeRowWidget.h"
#include "UI/WidgetController/AuraAttributeMenuController.h"
#include "AbilitySystem/Data/AttributeInfo.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"

/* =========================================================================================
 *	控件树
 * ========================================================================================= */

void UAuraAttributeMenuWidget::BuildWidgetTree()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	/* ① 全屏遮罩：点空白处关闭；用 Button 做，才能真的把点击吃掉，不让它穿到游戏里 */
	{
		UButton* Scrim = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Scrim"));
		const FSlateBrush ScrimBrush = FAuraUIStyle::Rounded(FAuraUIStyle::Scrim(), 0.f);
		FButtonStyle ScrimStyle;
		ScrimStyle.SetNormal(ScrimBrush);
		ScrimStyle.SetHovered(ScrimBrush);
		ScrimStyle.SetPressed(ScrimBrush);
		Scrim->SetStyle(ScrimStyle);
		Scrim->OnClicked.AddDynamic(this, &UAuraAttributeMenuWidget::HandleCloseClicked);

		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Scrim);
		PanelSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		PanelSlot->SetOffsets(FMargin(0.f));
	}

	/* ② 居中的面板 */
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuPanel"));
	Panel->SetBrush(FAuraUIStyle::RoundedOutlined(FAuraUIStyle::PanelFill(), 14.f, FAuraUIStyle::PanelOutline(), 2.f));
	Panel->SetPadding(FMargin(24.f, 18.f));
	{
		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetPosition(FVector2D::ZeroVector);
		PanelSlot->SetAutoSize(true);
	}

	UVerticalBox* PanelBox = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(PanelBox);

	/* --- 标题栏 --- */
	{
		UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>();
		PanelBox->AddChildToVerticalBox(HeaderRow)->SetPadding(FMargin(2.f, 0.f, 0.f, 12.f));

		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
		FAuraUIStyle::StyleText(Title, 26, FAuraUIStyle::Accent(), true);
		Title->SetText(FText::FromString(TEXT("属性")));
		{
			UHorizontalBoxSlot* PanelSlot = HeaderRow->AddChildToHorizontalBox(Title);
			PanelSlot->SetVerticalAlignment(VAlign_Center);
			PanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		UButton* CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
		CloseButton->SetStyle(FAuraUIStyle::ButtonStyle());
		CloseButton->OnClicked.AddDynamic(this, &UAuraAttributeMenuWidget::HandleCloseClicked);
		{
			UTextBlock* CloseLabel = WidgetTree->ConstructWidget<UTextBlock>();
			FAuraUIStyle::StyleText(CloseLabel, 15, FAuraUIStyle::TextPrimary(), true);
			CloseLabel->SetText(FText::FromString(TEXT("关闭")));
			CloseButton->SetContent(CloseLabel);
		}
		{
			UHorizontalBoxSlot* PanelSlot = HeaderRow->AddChildToHorizontalBox(CloseButton);
			PanelSlot->SetVerticalAlignment(VAlign_Center);
			PanelSlot->SetPadding(FMargin(14.f, 0.f, 0.f, 0.f));
		}
	}

	/* --- 分隔线 --- */
	{
		USizeBox* DividerBox = WidgetTree->ConstructWidget<USizeBox>();
		DividerBox->SetHeightOverride(1.f);

		UBorder* Divider = WidgetTree->ConstructWidget<UBorder>();
		Divider->SetBrush(FAuraUIStyle::Rounded(FAuraUIStyle::Divider(), 0.5f));
		DividerBox->AddChild(Divider);

		PanelBox->AddChildToVerticalBox(DividerBox)->SetPadding(FMargin(2.f, 0.f, 2.f, 12.f));
	}

	/* --- 属性列表 --- */
	{
		USizeBox* ListSize = WidgetTree->ConstructWidget<USizeBox>();
		ListSize->SetWidthOverride(620.f);
		ListSize->SetHeightOverride(500.f);

		UBorder* ListFrame = WidgetTree->ConstructWidget<UBorder>();
		ListFrame->SetBrush(FAuraUIStyle::Rounded(FLinearColor(0.f, 0.f, 0.f, 0.30f), 9.f));
		ListFrame->SetPadding(FMargin(10.f));
		ListSize->AddChild(ListFrame);

		AttributeScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("AttributeScroll"));
		AttributeScroll->SetScrollBarVisibility(ESlateVisibility::Visible);
		AttributeScroll->SetAllowOverscroll(false);
		ListFrame->SetContent(AttributeScroll);

		AttributeList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AttributeList"));
		AttributeScroll->AddChild(AttributeList);

		PanelBox->AddChildToVerticalBox(ListSize);
	}

	/* --- 底部提示 --- */
	{
		UTextBlock* Hint = WidgetTree->ConstructWidget<UTextBlock>();
		FAuraUIStyle::StyleText(Hint, 12, FAuraUIStyle::TextMuted(), false);
		Hint->SetText(FText::FromString(TEXT("点击面板外的区域或右上角按钮关闭")));
		PanelBox->AddChildToVerticalBox(Hint)->SetPadding(FMargin(2.f, 10.f, 0.f, 0.f));
	}
}

/* =========================================================================================
 *	数据
 * ========================================================================================= */

bool UAuraAttributeMenuWidget::BindWidgetDelegates()
{
	UAuraAttributeMenuController* Controller = Cast<UAuraAttributeMenuController>(WidgetController);
	if (!Controller) return false;   // 控制器还没给过来，基类会在之后重试

	Controller->AttributeInfoDelegate.AddDynamic(this, &UAuraAttributeMenuWidget::HandleAttributeInfo);
	return true;
}

void UAuraAttributeMenuWidget::HandleAttributeInfo(const FAuraAttributeInfo& Info)
{
	/* 已经有的属性：只刷新数值，不重建列表（数值变化非常频繁） */
	if (TObjectPtr<UAuraAttributeRowWidget>* ExistingRow = RowWidgets.Find(Info.AttributeTag))
	{
		if (*ExistingRow)
		{
			(*ExistingRow)->SetAttributeInfo(Info);
		}

		for (FAuraAttributeInfo& Cached : CachedAttributes)
		{
			if (Cached.AttributeTag == Info.AttributeTag)
			{
				Cached.AttributeValue = Info.AttributeValue;
				break;
			}
		}
		return;
	}

	/* 新属性：加进缓存再重建（属性的种类在开局就固定了，重建次数极少） */
	CachedAttributes.Add(Info);
	RebuildAttributeList();
}

void UAuraAttributeMenuWidget::RebuildAttributeList()
{
	if (!AttributeList || !WidgetTree) return;

	AttributeList->ClearChildren();
	RowWidgets.Reset();

	/* 组内按 Tag 名排序，保证每次打开顺序一致 */
	CachedAttributes.Sort([](const FAuraAttributeInfo& A, const FAuraAttributeInfo& B)
		{
			return A.AttributeTag.ToString() < B.AttributeTag.ToString();
		});

	/* 分组依据：父 Tag 在 Tag 树里天然存在（Attributes.Primary.Strength 会带出 Attributes.Primary） */
	struct FAttributeGroup
	{
		const TCHAR* ParentTag;
		const TCHAR* Title;
	};

	static const FAttributeGroup Groups[] =
	{
		{ TEXT("Attributes.Primary"),   TEXT("主属性") },
		{ TEXT("Attributes.Secondary"), TEXT("次要属性") },
		{ TEXT("Attributes.Vital"),     TEXT("生命与魔力") },
	};

	TSet<FGameplayTag> HandledTags;

	for (const FAttributeGroup& Group : Groups)
	{
		const FGameplayTag ParentTag = FGameplayTag::RequestGameplayTag(FName(Group.ParentTag), /*ErrorIfNotFound=*/false);
		if (!ParentTag.IsValid()) continue;

		TArray<const FAuraAttributeInfo*> GroupItems;
		for (const FAuraAttributeInfo& Cached : CachedAttributes)
		{
			if (Cached.AttributeTag.MatchesTag(ParentTag))
			{
				GroupItems.Add(&Cached);
			}
		}

		if (GroupItems.Num() == 0) continue;

		AddSectionHeader(FText::FromString(Group.Title));

		for (const FAuraAttributeInfo* Info : GroupItems)
		{
			UAuraAttributeRowWidget* Row = WidgetTree->ConstructWidget<UAuraAttributeRowWidget>();
			Row->SetAttributeInfo(*Info);

			AttributeList->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
			RowWidgets.Add(Info->AttributeTag, Row);
			HandledTags.Add(Info->AttributeTag);
		}
	}

	/* 兜底：不属于上面任何一组的属性也照样列出来 */
	TArray<const FAuraAttributeInfo*> Others;
	for (const FAuraAttributeInfo& Cached : CachedAttributes)
	{
		if (!HandledTags.Contains(Cached.AttributeTag))
		{
			Others.Add(&Cached);
		}
	}

	if (Others.Num() > 0)
	{
		AddSectionHeader(FText::FromString(TEXT("其它")));

		for (const FAuraAttributeInfo* Info : Others)
		{
			UAuraAttributeRowWidget* Row = WidgetTree->ConstructWidget<UAuraAttributeRowWidget>();
			Row->SetAttributeInfo(*Info);

			AttributeList->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
			RowWidgets.Add(Info->AttributeTag, Row);
		}
	}
}

void UAuraAttributeMenuWidget::AddSectionHeader(const FText& Title)
{
	if (!AttributeList) return;

	UTextBlock* Header = WidgetTree->ConstructWidget<UTextBlock>();
	FAuraUIStyle::StyleText(Header, 15, FAuraUIStyle::Accent(), true);
	Header->SetText(Title);
	AttributeList->AddChildToVerticalBox(Header)->SetPadding(FMargin(4.f, 10.f, 0.f, 6.f));

	USizeBox* DividerBox = WidgetTree->ConstructWidget<USizeBox>();
	DividerBox->SetHeightOverride(1.f);

	UBorder* Divider = WidgetTree->ConstructWidget<UBorder>();
	Divider->SetBrush(FAuraUIStyle::Rounded(FAuraUIStyle::Divider(), 0.5f));
	DividerBox->AddChild(Divider);

	AttributeList->AddChildToVerticalBox(DividerBox)->SetPadding(FMargin(4.f, 0.f, 4.f, 8.f));
}

void UAuraAttributeMenuWidget::HandleCloseClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AuraHUD->CloseAttributeMenu();
		}
	}
}
