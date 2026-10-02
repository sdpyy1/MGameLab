// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraEnemyHealthBarWidget.generated.h"

class UProgressBar;
class UOverlay;

/**
 *	敌人头顶的世界空间血条（纯 C++ 搭建）。
 *
 *	数据来源：订阅 AAuraEnemy 广播的 OnHealthChanged / OnMaxHealthChanged 委托（事件驱动）。
 *	订阅时机由基类的 TryBindDelegates() 保证：控件树和控制器谁先到都能订上；
 *	订阅成功时再主动拉一次当前值，保证出生时血条就是满的。
 *
 *	NativeTick 只负责"怎么画"，不碰数据。
 *
 *	受击表现：
 *	  · 白色闪光层：掉血瞬间整条发亮，随后快速衰减
 *	  · 掉血残影：主体条掉得快，残影条慢一步跟上，能直观看出"这一下掉了多少"
 *	  · 顿挫：整条轻微放大后回弹
 */
UCLASS()
class AURA_API UAuraEnemyHealthBarWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	virtual void BuildWidgetTree() override;
	virtual bool BindWidgetDelegates() override;

	/* ---------- 敌人的委托 ---------- */
	UFUNCTION() void HandleHealthChanged(float NewValue);
	UFUNCTION() void HandleMaxHealthChanged(float NewValue);

	UPROPERTY() TObjectPtr<UOverlay> RootOverlay;
	UPROPERTY() TObjectPtr<UProgressBar> HealthGhostBar;
	UPROPERTY() TObjectPtr<UProgressBar> HealthBar;
	UPROPERTY() TObjectPtr<UProgressBar> FlashBar;

	/* 目标值 / 显示值 / 残影值 */
	float TargetHealth = 0.f;
	float MaxHealth = 1.f;
	float DisplayHealth = 0.f;
	float GhostHealth = 0.f;

	/* 受击闪光剩余量 1 → 0 */
	float FlashAlpha = 0.f;

	/* 顿挫剩余时间 */
	float PunchTime = 0.f;

	static constexpr float FlashDuration = 0.35f;
	static constexpr float PunchDuration = 0.22f;
};
