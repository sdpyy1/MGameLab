// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AuraDamageTextWidget.h"

#include "UI/AuraUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"

void UAuraDamageTextWidget::BuildWidgetTree()
{
	/* 根节点铺满 DrawSize，文字居中；上飘靠 RenderTranslation 实现 */
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("DamageTextRoot"));
	WidgetTree->RootWidget = Root;

	DamageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DamageText"));
	FAuraUIStyle::StyleText(DamageText, 34, FAuraUIStyle::Accent(), true);

	/* 伤害数字用醒目的暖金色 + 深色描边阴影，任何背景下都读得清 */
	DamageText->SetShadowOffset(FVector2D(2.f, 2.f));
	DamageText->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.95f));
	DamageText->SetJustification(ETextJustify::Center);

	UOverlaySlot* PanelSlot = Root->AddChildToOverlay(DamageText);
	PanelSlot->SetHorizontalAlignment(HAlign_Center);
	PanelSlot->SetVerticalAlignment(VAlign_Center);
}

void UAuraDamageTextWidget::NativeConstruct()
{
	Super::NativeConstruct();

	/* 控件树已就绪，把可能提前传进来的伤害值写进去 */
	ApplyDamageText();
}

void UAuraDamageTextWidget::ApplyDamageText()
{
	if (DamageText)
	{
		DamageText->SetText(FText::FromString(FString::FromInt(FMath::RoundToInt(DamageValue))));
	}
}

void UAuraDamageTextWidget::SetDamageText(float Damage)
{
	DamageValue = FMath::Max(Damage, 0.f);

	/* 重开动画，并给一个随机偏移 / 角度，连续命中时不会糊成一团 */
	ElapsedTime = 0.f;
	RandomOffset = FVector2D(FMath::FRandRange(-26.f, 26.f), FMath::FRandRange(-10.f, 10.f));
	RandomAngle = FMath::FRandRange(-7.f, 7.f);

	ApplyDamageText();
}

void UAuraDamageTextWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ElapsedTime += InDeltaTime;

	const float Progress = FMath::Clamp(ElapsedTime / LifeTime, 0.f, 1.f);

	/* ① 上飘：先快后慢 */
	const float Rise = FMath::InterpEaseOut(0.f, RiseDistance, Progress, 2.2f);
	SetRenderTranslation(RandomOffset + FVector2D(0.f, -Rise));
	SetRenderTransformAngle(RandomAngle);

	/* ② 开头 0.16 秒弹一下：1.7 倍缩回 1 倍 */
	constexpr float PunchWindow = 0.16f;
	const float PunchRatio = FMath::Clamp(ElapsedTime / PunchWindow, 0.f, 1.f);
	const float Scale = FMath::Lerp(1.7f, 1.f, FMath::Pow(PunchRatio, 0.55f));
	SetRenderScale(FVector2D(Scale));

	/* ③ 后半段淡出 */
	constexpr float FadeStart = 0.55f;
	const float Alpha = (Progress < FadeStart) ? 1.f : 1.f - (Progress - FadeStart) / (1.f - FadeStart);
	SetRenderOpacity(FMath::Clamp(Alpha, 0.f, 1.f));
}
