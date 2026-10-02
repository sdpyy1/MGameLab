// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AuraOverlayWidget.h"

#include "UI/AuraUIStyle.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraMessageWidget.h"
#include "Character/AuraCharacterBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"

/* =========================================================================================
 *	控件树
 * ========================================================================================= */

UWidget* UAuraOverlayWidget::MakeBar(const FString& NamePrefix, const FLinearColor& FillColor, const FLinearColor& GhostColor,
	float Width, float Height, float CornerRadius,
	TObjectPtr<UProgressBar>& OutGhostBar, TObjectPtr<UProgressBar>& OutMainBar, TObjectPtr<UTextBlock>& OutValueText)
{
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_Box"), *NamePrefix));
	SizeBox->SetWidthOverride(Width);
	SizeBox->SetHeightOverride(Height);

	UOverlay* BarOverlay = WidgetTree->ConstructWidget<UOverlay>();
	SizeBox->AddChild(BarOverlay);

	/* ① 掉血残影：压在最底下，掉血后慢慢跟上 */
	OutGhostBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), *FString::Printf(TEXT("%s_Ghost"), *NamePrefix));
	OutGhostBar->SetWidgetStyle(FAuraUIStyle::BarStyle(GhostColor, CornerRadius));
	OutGhostBar->SetPercent(1.f);
	{
		UOverlaySlot* PanelSlot = BarOverlay->AddChildToOverlay(OutGhostBar);
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}

	/* ② 主体条：背景透明，这样残影能从没填满的部分透出来 */
	OutMainBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), *FString::Printf(TEXT("%s_Main"), *NamePrefix));
	OutMainBar->SetWidgetStyle(FAuraUIStyle::BarStyle(FillColor, CornerRadius));
	OutMainBar->SetPercent(1.f);
	{
		UOverlaySlot* PanelSlot = BarOverlay->AddChildToOverlay(OutMainBar);
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}

	/* ③ 数值文字：右对齐贴在条里 */
	OutValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%s_Text"), *NamePrefix));
	FAuraUIStyle::StyleText(OutValueText, FMath::Clamp(FMath::RoundToInt(Height * 0.64f), 10, 20), FAuraUIStyle::TextPrimary(), true);
	OutValueText->SetJustification(ETextJustify::Right);
	{
		UOverlaySlot* PanelSlot = BarOverlay->AddChildToOverlay(OutValueText);
		PanelSlot->SetHorizontalAlignment(HAlign_Right);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
		PanelSlot->SetPadding(FMargin(0.f, 0.f, 9.f, 0.f));
	}

	/* ④ 描边：浮在最上层，只画框不填充 */
	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>();
	Frame->SetBrush(FAuraUIStyle::RoundedOutlined(FLinearColor::Transparent, CornerRadius, FAuraUIStyle::BarOutline(), 1.5f));
	{
		UOverlaySlot* PanelSlot = BarOverlay->AddChildToOverlay(Frame);
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}

	return SizeBox;
}

void UAuraOverlayWidget::BuildWidgetTree()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	/* ============================ 左下角：等级 + 生命 / 魔力 ============================ */

	UBorder* StatPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StatPanel"));
	StatPanel->SetBrush(FAuraUIStyle::RoundedOutlined(FAuraUIStyle::PanelFill(), 12.f, FAuraUIStyle::PanelOutline(), 1.5f));
	StatPanel->SetPadding(FMargin(16.f, 12.f));
	StatPanel->SetVerticalAlignment(VAlign_Center);
	{
		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(StatPanel);
		PanelSlot->SetAnchors(FAnchors(0.f, 1.f));
		PanelSlot->SetAlignment(FVector2D(0.f, 1.f));
		PanelSlot->SetPosition(FVector2D(36.f, -36.f));
		PanelSlot->SetAutoSize(true);
	}

	UHorizontalBox* StatRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	StatPanel->SetContent(StatRow);

	/* --- 等级徽章 --- */
	{
		USizeBox* BadgeBox = WidgetTree->ConstructWidget<USizeBox>();
		BadgeBox->SetWidthOverride(46.f);
		BadgeBox->SetHeightOverride(46.f);

		UBorder* Badge = WidgetTree->ConstructWidget<UBorder>();
		Badge->SetBrush(FAuraUIStyle::RoundedOutlined(FLinearColor(0.10f, 0.09f, 0.06f, 0.95f), 10.f, FAuraUIStyle::Accent(), 1.5f));
		Badge->SetHorizontalAlignment(HAlign_Center);
		Badge->SetVerticalAlignment(VAlign_Center);
		BadgeBox->AddChild(Badge);

		LevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelText"));
		FAuraUIStyle::StyleText(LevelText, 22, FAuraUIStyle::Accent(), true);
		LevelText->SetText(FText::FromString(TEXT("1")));
		Badge->SetContent(LevelText);

		UHorizontalBoxSlot* BadgePanelSlot = StatRow->AddChildToHorizontalBox(BadgeBox);
		BadgePanelSlot->SetVerticalAlignment(VAlign_Center);
		BadgePanelSlot->SetPadding(FMargin(0.f, 0.f, 14.f, 0.f));
	}

	/* --- 生命条 / 魔力条 --- */
	{
		UVerticalBox* Bars = WidgetTree->ConstructWidget<UVerticalBox>();

		UWidget* HealthBlock = MakeBar(TEXT("Health"), FAuraUIStyle::HealthFill(), FAuraUIStyle::HealthGhost(),
			340.f, 24.f, 12.f, HealthGhostBar, HealthBar, HealthValueText);
		UWidget* ManaBlock = MakeBar(TEXT("Mana"), FAuraUIStyle::ManaFill(), FAuraUIStyle::ManaGhost(),
			340.f, 18.f, 9.f, ManaGhostBar, ManaBar, ManaValueText);

		Bars->AddChildToVerticalBox(HealthBlock)->SetPadding(FMargin(0.f, 0.f, 0.f, 7.f));
		Bars->AddChildToVerticalBox(ManaBlock);

		StatRow->AddChildToHorizontalBox(Bars)->SetVerticalAlignment(VAlign_Center);
	}

	/* ============================ 右侧中间：消息通知列表 ============================ */

	/*
		锚在屏幕右侧中间，并以自身右下角对齐锚点：
		消息越堆越多时，整列会以屏幕垂直中线为中心向上下两侧一起长，不会跑到屏幕外。
	*/
	MessageList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MessageList"));
	{
		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(MessageList);
		PanelSlot->SetAnchors(FAnchors(1.f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(1.f, 0.5f));
		PanelSlot->SetPosition(FVector2D(-40.f, 0.f));
		PanelSlot->SetAutoSize(true);
	}

	/* ============================ 右下角：属性菜单按钮 ============================ */

	UButton* MenuButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("AttributeMenuButton"));
	MenuButton->SetStyle(FAuraUIStyle::ButtonStyle());
	MenuButton->OnClicked.AddDynamic(this, &UAuraOverlayWidget::HandleMenuButtonClicked);
	{
		UTextBlock* MenuLabel = WidgetTree->ConstructWidget<UTextBlock>();
		FAuraUIStyle::StyleText(MenuLabel, 17, FAuraUIStyle::Accent(), true);
		MenuLabel->SetText(FText::FromString(TEXT("属性菜单")));
		MenuButton->SetContent(MenuLabel);
	}
	{
		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(MenuButton);
		PanelSlot->SetAnchors(FAnchors(1.f, 1.f));
		PanelSlot->SetAlignment(FVector2D(1.f, 1.f));
		PanelSlot->SetPosition(FVector2D(-36.f, -36.f));
		PanelSlot->SetAutoSize(true);
	}
}

/* =========================================================================================
 *	委托与刷新
 * ========================================================================================= */

bool UAuraOverlayWidget::BindWidgetDelegates()
{
	UAuraOverlayWidgetController* Controller = Cast<UAuraOverlayWidgetController>(WidgetController);
	if (!Controller) return false;   // 控制器还没给过来，基类会在之后重试

	/* 等级徽章：直接问角色要（等级统一放在 AAuraCharacterBase 上） */
	if (AAuraCharacterBase* Character = Cast<AAuraCharacterBase>(GetOwningPlayerPawn()))
	{
		if (LevelText)
		{
			LevelText->SetText(FText::FromString(FString::FromInt(Character->GetPlayerLevel())));
		}
	}

	Controller->OnHealthChanged.AddDynamic(this, &UAuraOverlayWidget::HandleHealthChanged);
	Controller->OnMaxHealthChanged.AddDynamic(this, &UAuraOverlayWidget::HandleMaxHealthChanged);
	Controller->OnManaChanged.AddDynamic(this, &UAuraOverlayWidget::HandleManaChanged);
	Controller->OnMaxManaChanged.AddDynamic(this, &UAuraOverlayWidget::HandleMaxManaChanged);
	Controller->MessageWidgetRowDelegate.AddDynamic(this, &UAuraOverlayWidget::HandleMessageRow);

	return true;
}

void UAuraOverlayWidget::HandleHealthChanged(float NewValue)
{
	TargetHealth = NewValue;
}

void UAuraOverlayWidget::HandleMaxHealthChanged(float NewValue)
{
	MaxHealth = FMath::Max(NewValue, KINDA_SMALL_NUMBER);
}

void UAuraOverlayWidget::HandleManaChanged(float NewValue)
{
	TargetMana = NewValue;
}

void UAuraOverlayWidget::HandleMaxManaChanged(float NewValue)
{
	MaxMana = FMath::Max(NewValue, KINDA_SMALL_NUMBER);
}

void UAuraOverlayWidget::HandleMessageRow(FUIWidgetRow Row)
{
	/*
		每来一条消息就新加一条通知，全部堆在右侧列表里依次排开。
		注意不要复用同一条：短时间内触发多条时，旧的那条不能被覆盖掉。
	*/
	if (!MessageList || !WidgetTree) return;

	UAuraMessageWidget* Message = WidgetTree->ConstructWidget<UAuraMessageWidget>();
	if (!Message) return;

	UVerticalBoxSlot* PanelSlot = MessageList->AddChildToVerticalBox(Message);
	PanelSlot->SetHorizontalAlignment(HAlign_Right);
	PanelSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

	Message->SetMessage(Row.Message, Row.Image);
	ActiveMessages.Add(Message);
}

void UAuraOverlayWidget::HandleMenuButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AuraHUD->ToggleAttributeMenu();
		}
	}
}

void UAuraOverlayWidget::RefreshBar(UProgressBar* MainBar, UProgressBar* GhostBar, UTextBlock* ValueText,
	float DisplayValue, float GhostValue, float MaxValue)
{
	const float SafeMax = FMath::Max(MaxValue, KINDA_SMALL_NUMBER);
	const float MainPercent = FMath::Clamp(DisplayValue / SafeMax, 0.f, 1.f);
	const float GhostPercent = FMath::Clamp(GhostValue / SafeMax, 0.f, 1.f);

	if (MainBar) MainBar->SetPercent(MainPercent);
	if (GhostBar) GhostBar->SetPercent(FMath::Max(MainPercent, GhostPercent));

	if (ValueText)
	{
		ValueText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"),
			FMath::RoundToInt(DisplayValue), FMath::RoundToInt(MaxValue))));
	}
}

void UAuraOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	/*
		主体条快速逼近目标值；残影条只在"掉血"时慢速跟随，
		加血 / 满血恢复时直接跟上，免得出现反方向乱窜的残影。
	*/
	DisplayHealth = FMath::FInterpTo(DisplayHealth, TargetHealth, InDeltaTime, 12.f);
	GhostHealth = (GhostHealth < DisplayHealth) ? DisplayHealth : FMath::FInterpTo(GhostHealth, DisplayHealth, InDeltaTime, 3.f);
	RefreshBar(HealthBar, HealthGhostBar, HealthValueText, DisplayHealth, GhostHealth, MaxHealth);

	DisplayMana = FMath::FInterpTo(DisplayMana, TargetMana, InDeltaTime, 12.f);
	GhostMana = (GhostMana < DisplayMana) ? DisplayMana : FMath::FInterpTo(GhostMana, DisplayMana, InDeltaTime, 3.f);
	RefreshBar(ManaBar, ManaGhostBar, ManaValueText, DisplayMana, GhostMana, MaxMana);

	/*
		回收播完的消息通知。
		统一在这里移除（而不是让每条消息在自己的 Tick 里 RemoveFromParent），
		避免在遍历控件树的过程中改动控件树。
	*/
	for (int32 Index = ActiveMessages.Num() - 1; Index >= 0; --Index)
	{
		UAuraMessageWidget* Message = ActiveMessages[Index];
		if (!Message)
		{
			ActiveMessages.RemoveAt(Index);
			continue;
		}

		if (Message->IsFinished())
		{
			Message->RemoveFromParent();
			ActiveMessages.RemoveAt(Index);
		}
	}
}
