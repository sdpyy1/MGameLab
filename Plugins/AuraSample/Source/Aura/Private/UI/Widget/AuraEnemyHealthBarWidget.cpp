// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AuraEnemyHealthBarWidget.h"

#include "UI/AuraUIStyle.h"
#include "Character/AuraEnemy.h"
#include "AbilitySystem/AuraAttributeSet.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"

void UAuraEnemyHealthBarWidget::BuildWidgetTree()
{
	/* 根节点铺满整个 DrawSize，血条在里面居中，这样顿挫放大时不会被裁掉 */
	RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
	WidgetTree->RootWidget = RootOverlay;

	USizeBox* BarBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("BarBox"));
	BarBox->SetWidthOverride(150.f);
	BarBox->SetHeightOverride(20.f);
	{
		UOverlaySlot* PanelSlot = RootOverlay->AddChildToOverlay(BarBox);
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
	}

	UOverlay* BarOverlay = WidgetTree->ConstructWidget<UOverlay>();
	BarBox->AddChild(BarOverlay);

	/* ① 掉血残影 */
	HealthGhostBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthGhostBar"));
	HealthGhostBar->SetWidgetStyle(FAuraUIStyle::BarStyle(FLinearColor(1.f, 0.72f, 0.35f, 0.95f), 10.f));
	HealthGhostBar->SetPercent(1.f);
	{
		UOverlaySlot* PanelSlot = BarOverlay->AddChildToOverlay(HealthGhostBar);
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}

	/* ② 主体血条 */
	HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
	HealthBar->SetWidgetStyle(FAuraUIStyle::BarStyle(FAuraUIStyle::EnemyHealthFill(), 10.f));
	HealthBar->SetPercent(1.f);
	{
		UOverlaySlot* PanelSlot = BarOverlay->AddChildToOverlay(HealthBar);
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}

	/* ③ 受击闪光层：平时完全透明 */
	FlashBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("FlashBar"));
	FlashBar->SetWidgetStyle(FAuraUIStyle::BarStyle(FLinearColor::White, 10.f));
	FlashBar->SetPercent(1.f);
	FlashBar->SetRenderOpacity(0.f);
	{
		UOverlaySlot* PanelSlot = BarOverlay->AddChildToOverlay(FlashBar);
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}

	/* ④ 描边 */
	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>();
	Frame->SetBrush(FAuraUIStyle::RoundedOutlined(FLinearColor::Transparent, 10.f, FAuraUIStyle::BarOutline(), 1.5f));
	{
		UOverlaySlot* PanelSlot = BarOverlay->AddChildToOverlay(Frame);
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

bool UAuraEnemyHealthBarWidget::BindWidgetDelegates()
{
	AAuraEnemy* Enemy = Cast<AAuraEnemy>(WidgetController);
	if (!Enemy) return false;   // 控制器还没给过来，基类会在之后重试

	Enemy->OnHealthChangedDelegate.AddDynamic(this, &UAuraEnemyHealthBarWidget::HandleHealthChanged);
	Enemy->OnMaxHealthChangedDelegate.AddDynamic(this, &UAuraEnemyHealthBarWidget::HandleMaxHealthChanged);

	/*
		敌人的初始广播发生在 BeginPlay，可能早于本控件订阅，
		所以这里主动拉一次当前值：出生时血条直接是满的，而不是从 0 涨上来。
	*/
	if (const UAuraAttributeSet* EnemyAttributes = Enemy->GetAttributeSet())
	{
		MaxHealth = FMath::Max(EnemyAttributes->GetMaxHealth(), KINDA_SMALL_NUMBER);
		TargetHealth = DisplayHealth = GhostHealth = EnemyAttributes->GetHealth();
	}

	return true;
}

void UAuraEnemyHealthBarWidget::HandleMaxHealthChanged(float NewValue)
{
	MaxHealth = FMath::Max(NewValue, KINDA_SMALL_NUMBER);
}

void UAuraEnemyHealthBarWidget::HandleHealthChanged(float NewValue)
{
	/* 掉血 → 触发闪光 + 顿挫；加血则不触发 */
	if (NewValue < TargetHealth)
	{
		FlashAlpha = 1.f;
		PunchTime = PunchDuration;
	}

	TargetHealth = NewValue;
}

void UAuraEnemyHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	/* 这里只负责"怎么画"，数值由委托推进来 */

	/* 主体条快速跟随，残影条慢速追赶（只在掉血方向，加血直接跟上） */
	DisplayHealth = FMath::FInterpTo(DisplayHealth, TargetHealth, InDeltaTime, 16.f);
	GhostHealth = (GhostHealth < DisplayHealth) ? DisplayHealth : FMath::FInterpTo(GhostHealth, DisplayHealth, InDeltaTime, 3.f);

	const float MainPercent = FMath::Clamp(DisplayHealth / MaxHealth, 0.f, 1.f);

	if (HealthBar) HealthBar->SetPercent(MainPercent);
	if (HealthGhostBar) HealthGhostBar->SetPercent(FMath::Max(MainPercent, FMath::Clamp(GhostHealth / MaxHealth, 0.f, 1.f)));
	if (FlashBar) FlashBar->SetPercent(MainPercent);

	/* 闪光衰减：前段亮、后段快速收干净 */
	if (FlashAlpha > 0.f)
	{
		FlashAlpha = FMath::Max(0.f, FlashAlpha - InDeltaTime / FlashDuration);
		if (FlashBar)
		{
			FlashBar->SetRenderOpacity(FMath::Pow(FlashAlpha, 0.7f) * 0.9f);
		}
	}

	/* 顿挫：整条轻微放大后回弹（二次曲线，收尾更干脆） */
	const float PunchRatio = FMath::Clamp(PunchTime / PunchDuration, 0.f, 1.f);
	SetRenderScale(FVector2D(1.f + 0.10f * PunchRatio * PunchRatio));

	if (PunchTime > 0.f)
	{
		PunchTime = FMath::Max(0.f, PunchTime - InDeltaTime);
	}
}
