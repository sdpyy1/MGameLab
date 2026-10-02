// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/DamageTextWidgetComponent.h"

#include "UI/Widget/AuraDamageTextWidget.h"
#include "TimerManager.h"

UDamageTextWidgetComponent::UDamageTextWidgetComponent()
{
	/* 屏幕空间：始终正对镜头、大小恒定，适合飘字 */
	SetWidgetSpace(EWidgetSpace::Screen);

	/* 留足空间给"上飘 + 放大"，否则数字会在边缘被裁掉 */
	SetDrawSize(FVector2D(260.f, 230.f));
	SetPivot(FVector2D(0.5f, 0.5f));
	SetTwoSided(true);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);

	/* 纯 C++ 控件类：不依赖 WBP */
	WidgetClass = UAuraDamageTextWidget::StaticClass();
}

void UDamageTextWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	/*
		强制指定控件类：即使 BP 子类里配了旧的 WBP，也以这里为准。
		SetWidgetClass 会重建内部控件，所以必须放在 SetDamageText 之前。
	*/
	SetWidgetClass(UAuraDamageTextWidget::StaticClass());

	/* 动画播完就回收（组件没有 SetLifeSpan，用世界计时器到点销毁自己） */
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DestroyTimerHandle, this, &UDamageTextWidgetComponent::HandleLifeTimeElapsed, LifeTime, false);
	}
}

void UDamageTextWidgetComponent::HandleLifeTimeElapsed()
{
	DestroyComponent();
}

void UDamageTextWidgetComponent::SetDamageText(float Damage)
{
	if (UAuraDamageTextWidget* DamageWidget = Cast<UAuraDamageTextWidget>(GetUserWidgetObject()))
	{
		DamageWidget->SetDamageText(Damage);
	}
}
